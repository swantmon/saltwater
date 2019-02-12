import argparse
import glob
import os
import numpy as np
import math
import sys
import random

import torchvision.transforms as transforms
from torchvision.utils import save_image
from PIL import Image

from torch.utils.data import DataLoader, Dataset
from torchvision import datasets
from torch.autograd import Variable
from multiprocessing import Process, freeze_support

from PIL import Image

from gans import *

import torch.nn as nn
import torch.nn.functional as F
import torch

# -----------------------------------------------------------------------------
# Config
# -----------------------------------------------------------------------------
parser = argparse.ArgumentParser()
parser.add_argument('--n_epochs', type=int, default=40000, help='number of epochs of training')
parser.add_argument('--batch_size', type=int, default=64, help='size of the batches')
parser.add_argument('--path_to_dataset', type=str, default='D:/NN/dataset/ILSVRC2012_img_train/', help='path to the dataset (no recursive search)')
parser.add_argument('--lr', type=float, default=0.0002, help='adam: learning rate')
parser.add_argument('--b1', type=float, default=0.5, help='adam: decay of first order momentum of gradient')
parser.add_argument('--b2', type=float, default=0.999, help='adam: decay of first order momentum of gradient')
parser.add_argument('--n_cpu', type=int, default=0, help='number of cpu threads to use during batch generation')
parser.add_argument('--img_size_w', type=int, default=128, help='size of each image dimension')
parser.add_argument('--img_size_h', type=int, default=128, help='size of each image dimension')
parser.add_argument('--img_channels', type=int, default=3, help='number of image channels')
parser.add_argument('--mask_size', type=int, default=64, help='size of random mask')
parser.add_argument('--sample_interval', type=int, default=100, help='interval between image sampling')
parser.add_argument('--output', type=str, default='D:/NN/plugin_slam/output/ILSVRC2012_img_train/', help='output folder of the results')
parser.add_argument('--path_to_savepoint', type=str, default='D:/NN/plugin_slam/savepoint/', help='path to load and store savepoint')
opt = parser.parse_args()

# -----------------------------------------------------------------------------
# Output
# -----------------------------------------------------------------------------
os.makedirs(opt.output, exist_ok=True)
os.makedirs(opt.path_to_savepoint, exist_ok=True)

# -----------------------------------------------------------------------------
# Dataset
# -----------------------------------------------------------------------------
class ImageDataset(Dataset):
    def __init__(self, root, transforms_=None, mode='train'):
        self.transform = transforms.Compose(transforms_)
        self.mode = mode
        self.files = sorted(glob.glob('%s/*.*' % root))
        self.files = self.files[:-4000] if mode == 'train' else self.files[-4000:]

    # -----------------------------------------------------------------------------

    def apply_train_mask(self, img):
        masked_img = img.clone()
        
        #y1 = np.random.randint(0, opt.img_size_h - opt.mask_size)
        #x1 = np.random.randint(0, opt.img_size_w - opt.mask_size)
        y1 = int((opt.img_size_h - opt.mask_size) / 2)
        x1 = int((opt.img_size_w - opt.mask_size) / 2)

        y2, x2 = y1 + opt.mask_size, x1 + opt.mask_size

        masked_img[:, y1:y2, x1:x2] = 1
        masked_part = img[:, y1:y2, x1:x2]

        return masked_img, masked_part, 0

    # -----------------------------------------------------------------------------

    def apply_test_mask(self, img):
        masked_img = img.clone()
        
        y1 = int((opt.img_size_h - opt.mask_size) / 2)
        x1 = int((opt.img_size_w - opt.mask_size) / 2)

        y2, x2 = y1 + opt.mask_size, x1 + opt.mask_size

        masked_img[:, y1:y2, x1:x2] = 1

        return masked_img, 0, ([x1, y1, x2, y2])

    # -----------------------------------------------------------------------------

    def __getitem__(self, index):
        img = Image.open(self.files[index % len(self.files)])
        img = img.convert('RGB')
        img = self.transform(img)

        if self.mode == 'train':
            # For training data perform random mask
            masked_img, masked_part, pos = self.apply_train_mask(img)
        else:
            # For test data mask the center of the image
            masked_img, masked_part, pos = self.apply_test_mask(img)

        return img, masked_img, masked_part, pos

    # -----------------------------------------------------------------------------

    def __len__(self):
        return len(self.files)

# -----------------------------------------------------------------------------
# Check if cuda is available
# -----------------------------------------------------------------------------
cuda = True if torch.cuda.is_available() else False

# -----------------------------------------------------------------------------
# Calculate output of image discriminator (PatchGAN)
# -----------------------------------------------------------------------------
patch_h, patch_w = int(opt.mask_size / 2**3), int(opt.mask_size / 2**3)
patch = (1, patch_h, patch_w)

# -----------------------------------------------------------------------------

def weights_init_normal(m):
    classname = m.__class__.__name__
    if classname.find('Conv') != -1:
        torch.nn.init.normal_(m.weight.data, 0.0, 0.02)
    elif classname.find('BatchNorm2d') != -1:
        torch.nn.init.normal_(m.weight.data, 1.0, 0.02)
        torch.nn.init.constant_(m.bias.data, 0.0)

# -----------------------------------------------------------------------------
# Loss function
# -----------------------------------------------------------------------------
adversarial_loss = torch.nn.MSELoss()
pixelwise_loss = torch.nn.L1Loss()

# -----------------------------------------------------------------------------
# Initialize generator and discriminator
# -----------------------------------------------------------------------------
generator = Generator(channels=opt.img_channels)
discriminator = Discriminator(channels=opt.img_channels)

# -----------------------------------------------------------------------------

if cuda:
    generator.cuda()
    discriminator.cuda()
    adversarial_loss.cuda()
    pixelwise_loss.cuda()

# -----------------------------------------------------------------------------
# Initialize weights
# -----------------------------------------------------------------------------
generator.apply(weights_init_normal)
discriminator.apply(weights_init_normal)

# -----------------------------------------------------------------------------
# Dataset loader
# -----------------------------------------------------------------------------
transforms_ = [ transforms.Resize((opt.img_size_h, opt.img_size_w), Image.BICUBIC),
                transforms.ToTensor(),
                transforms.Normalize((0.5,0.5,0.5), (0.5,0.5,0.5)) ]

# -----------------------------------------------------------------------------

train_dataloader = DataLoader(ImageDataset(opt.path_to_dataset, transforms_=transforms_),
                        batch_size=opt.batch_size, shuffle=True, num_workers=opt.n_cpu)

test_dataloader = DataLoader(ImageDataset(opt.path_to_dataset, transforms_=transforms_, mode='test'),
                        batch_size=12, shuffle=True, num_workers=opt.n_cpu)

# -----------------------------------------------------------------------------
# Optimizers
# -----------------------------------------------------------------------------
optimizer_G = torch.optim.Adam(generator.parameters(), lr=opt.lr, betas=(opt.b1, opt.b2))
optimizer_D = torch.optim.Adam(discriminator.parameters(), lr=opt.lr, betas=(opt.b1, opt.b2))

Tensor = torch.cuda.FloatTensor if cuda else torch.FloatTensor


# -----------------------------------------------------------------------------
# Functions
# -----------------------------------------------------------------------------
def save_sample(batches_done, _Path):
    samples, masked_samples, masked_part, position = next(iter(test_dataloader))
    samples = Variable(samples.type(Tensor))
    masked_samples = Variable(masked_samples.type(Tensor))

    x1 = position[0][0].item()
    y1 = position[1][0].item()
    x2 = position[2][0].item()
    y2 = position[3][0].item()

    # -----------------------------------------------------------------------------
    # Generate inpainted image
    # -----------------------------------------------------------------------------
    gen_mask = generator(masked_samples)

    filled_sample = masked_samples.clone()
    filled_sample[:, :, y1:y2, x1:x2] = gen_mask

    # -----------------------------------------------------------------------------
    # Save sample
    # -----------------------------------------------------------------------------
    sample = torch.cat((masked_samples.data, filled_sample.data, samples.data), -2)
    save_image(sample, _Path, nrow=6, normalize=True)

# -----------------------------------------------------------------------------
# Main function
# -----------------------------------------------------------------------------
if __name__ == '__main__':

    try:

        if os.path.isfile(opt.path_to_savepoint + '/model_best_generator.pth.tar'):
            Checkpoint = LoadCheckpoint(opt.path_to_savepoint + '/model_best_discriminator.pth.tar')

            discriminator.load_state_dict(Checkpoint['state_dict'])
            optimizer_D.load_state_dict(Checkpoint['optimizer'])

            # -----------------------------------------------------------------------------

            Checkpoint = LoadCheckpoint(opt.path_to_savepoint + '/model_best_generator.pth.tar')

            LastEpoch            = Checkpoint['epoch']
            GeneratorMinimalLoss = Checkpoint['best_prec1']

            generator.load_state_dict(Checkpoint['state_dict'])
            optimizer_G.load_state_dict(Checkpoint['optimizer'])

            print ("Loaded extisting checkpoint to resume in epoch", LastEpoch)
        else:
            LastEpoch = 0
            GeneratorMinimalLoss = 1000

        # -----------------------------------------------------------------------------

        for epoch in range(LastEpoch, opt.n_epochs):
            for i, (imgs, masked_imgs, masked_parts, position) in enumerate(train_dataloader):

                # -----------------------------------------------------------------------------
                # Adversarial ground truths
                # -----------------------------------------------------------------------------
                valid = Variable(Tensor(imgs.shape[0], *patch).fill_(1.0), requires_grad=False)
                fake = Variable(Tensor(imgs.shape[0], *patch).fill_(0.0), requires_grad=False)

                # -----------------------------------------------------------------------------
                # Configure input
                # -----------------------------------------------------------------------------
                imgs = Variable(imgs.type(Tensor))
                masked_imgs = Variable(masked_imgs.type(Tensor))
                masked_parts = Variable(masked_parts.type(Tensor))

                # -----------------------------------------------------------------------------
                #  Train Generator
                # -----------------------------------------------------------------------------
                optimizer_G.zero_grad()

                # -----------------------------------------------------------------------------
                # Generate a batch of images
                # -----------------------------------------------------------------------------
                gen_parts = generator(masked_imgs)

                # -----------------------------------------------------------------------------
                # Adversarial and pixelwise loss
                # -----------------------------------------------------------------------------
                g_adv = adversarial_loss(discriminator(gen_parts), valid)
                g_pixel = pixelwise_loss(gen_parts, masked_parts)

                g_loss = 0.001 * g_adv + 0.999 * g_pixel

                g_loss.backward()
                optimizer_G.step()

                # -----------------------------------------------------------------------------
                #  Train Discriminator
                # -----------------------------------------------------------------------------
                optimizer_D.zero_grad()

                # -----------------------------------------------------------------------------
                # Measure discriminator's ability to classify real from generated samples
                # -----------------------------------------------------------------------------
                real_loss = adversarial_loss(discriminator(masked_parts), valid)
                fake_loss = adversarial_loss(discriminator(gen_parts.detach()), fake)
                d_loss = 0.5 * (real_loss + fake_loss)

                d_loss.backward()
                optimizer_D.step()

                # -----------------------------------------------------------------------------
                # Generate sample at sample interval w/ test dataset
                # -----------------------------------------------------------------------------
                batches_done = epoch * len(train_dataloader) + i
                if batches_done % opt.sample_interval == 0:
                    save_sample(batches_done, opt.output + '/%d.png' % batches_done)

                # -----------------------------------------------------------------------------
                # Save network if it is the best up to now
                # -----------------------------------------------------------------------------
                if g_loss < GeneratorMinimalLoss and epoch > 0:
                    GeneratorMinimalLoss = g_loss
                    SaveCheckpoint(epoch, generator.state_dict(), g_loss, optimizer_G.state_dict(), opt.path_to_savepoint + '/model_best_generator.pth.tar')
                    SaveCheckpoint(epoch, discriminator.state_dict(), d_loss, optimizer_D.state_dict(), opt.path_to_savepoint + '/model_best_discriminator.pth.tar')
                    save_sample(batches_done, opt.path_to_savepoint + '/model_best_batch.png')
                
                # -----------------------------------------------------------------------------
                # Round end
                # -----------------------------------------------------------------------------
                print ('[Epoch %d/%d] [Batch %d/%d] [D loss: %f] [G adv: %f, pixel: %f, total: %f]' % (epoch, opt.n_epochs, i, len(train_dataloader), d_loss.item(), g_adv.item(), g_pixel.item(), g_loss.item()))

    except OSError as _Error:
        print("OS error: {0}".format(_Error))
    except ValueError:
        print("Could not convert data to an value.")
    except:
        print("Unexpected error:", sys.exc_info()[0])
        raise