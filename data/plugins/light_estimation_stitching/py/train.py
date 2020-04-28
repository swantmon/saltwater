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
parser.add_argument('--n_epochs', type=int, default=2000, help='number of epochs of training')
parser.add_argument('--batch_size', type=int, default=16, help='size of the batches')
parser.add_argument('--path_to_dataset', type=str, default='./dataset/', help='path to the dataset (no recursive search)')
parser.add_argument('--lr', type=float, default=0.0002, help='adam: learning rate')
parser.add_argument('--b1', type=float, default=0.5, help='adam: decay of first order momentum of gradient')
parser.add_argument('--b2', type=float, default=0.999, help='adam: decay of first order momentum of gradient')
parser.add_argument('--n_cpu', type=int, default=0, help='number of cpu threads to use during batch generation')
parser.add_argument('--img_size_w', type=int, default=256, help='size of each image dimension')
parser.add_argument('--img_size_h', type=int, default=128, help='size of each image dimension')
parser.add_argument('--img_channels', type=int, default=3, help='number of image channels')
parser.add_argument('--mask_ground_and_sky', type=float, default=0.3, help='Percentage of a mask for lower and upper part')
parser.add_argument('--number_of_masks', type=int, default=4, help='number of random mask')
parser.add_argument('--mask_size', type=int, default=64, help='size of random mask')
parser.add_argument('--sample_interval', type=int, default=5000, help='interval between image sampling')
parser.add_argument('--output', type=str, default='./output/', help='output folder of the results')
parser.add_argument('--path_to_savepoint', type=str, default='./savepoint/', help='path to load and store savepoint')
opt, unknown_opt = parser.parse_known_args()

# -----------------------------------------------------------------------------
# Output
# -----------------------------------------------------------------------------
os.makedirs(opt.output, exist_ok=True)
os.makedirs(opt.path_to_savepoint, exist_ok=True)

# -----------------------------------------------------------------------------
# Dataset
# -----------------------------------------------------------------------------
def generate_perlin_noise_2d(shape, res):
    def f(t):
        return 6*t**5 - 15*t**4 + 10*t**3
    
    delta = (res[0] / shape[0], res[1] / shape[1])
    d = (shape[0] // res[0], shape[1] // res[1])
    grid = np.mgrid[0:res[0]:delta[0],0:res[1]:delta[1]].transpose(1, 2, 0) % 1
    # Gradients
    angles = 2*np.pi*np.random.rand(res[0]+1, res[1]+1)
    gradients = np.dstack((np.cos(angles), np.sin(angles)))
    g00 = gradients[0:-1,0:-1].repeat(d[0], 0).repeat(d[1], 1)
    g10 = gradients[1:  ,0:-1].repeat(d[0], 0).repeat(d[1], 1)
    g01 = gradients[0:-1,1:  ].repeat(d[0], 0).repeat(d[1], 1)
    g11 = gradients[1:  ,1:  ].repeat(d[0], 0).repeat(d[1], 1)
    # Ramps
    n00 = np.sum(np.dstack((grid[:,:,0]  , grid[:,:,1]  )) * g00, 2)
    n10 = np.sum(np.dstack((grid[:,:,0]-1, grid[:,:,1]  )) * g10, 2)
    n01 = np.sum(np.dstack((grid[:,:,0]  , grid[:,:,1]-1)) * g01, 2)
    n11 = np.sum(np.dstack((grid[:,:,0]-1, grid[:,:,1]-1)) * g11, 2)
    # Interpolation
    t = f(grid)
    n0 = n00*(1-t[:,:,0]) + t[:,:,0]*n10
    n1 = n01*(1-t[:,:,0]) + t[:,:,0]*n11
    return np.sqrt(2)*((1-t[:,:,1])*n0 + t[:,:,1]*n1)
        
def generate_fractal_noise_2d(shape, res, octaves=1, persistence=0.5, frequence=2):
    noise = np.zeros(shape)
    frequency = 1
    amplitude = 1
    for _ in range(octaves):
        noise += amplitude * generate_perlin_noise_2d(shape, (frequency*res[0], frequency*res[1]))
        frequency *= frequence
        amplitude *= persistence
    return noise

class ImageDataset(Dataset):
    def __init__(self, root, transforms_=None, mode='train'):
        self.transform = transforms.Compose(transforms_)
        self.mode = mode
        self.files = sorted(glob.glob('%s/*.*' % root))
        self.files = self.files[:-4000] if mode == 'train' else self.files[-4000:]

    # -----------------------------------------------------------------------------

    def generateMaskTensor(self):
        noise = generate_fractal_noise_2d((128, 256), (2, 2), octaves=3, persistence=0.4, frequence=1)
        noise += generate_fractal_noise_2d((128, 256), (2, 2), octaves=1, persistence=4.0, frequence=1)
        noise *= 2.0
        noise = np.clip(noise, 0, 1)
        noise[noise <  0.5] = 0.0
        noise[noise >= 0.5] = 1.0

        return torch.Tensor(noise)

    # -----------------------------------------------------------------------------

    def apply_random_mask(self, img):
        masked_part = img.clone()
        masked_img = img.clone()

        noise = self.generateMaskTensor()

        masked_img = torch.ones(img.shape) * (1.0 - noise) + masked_img * noise

        return masked_img, masked_part

    # -----------------------------------------------------------------------------

    def apply_panorama_mask(self, img):
        masked_part = torch.zeros(img.shape)
        masked_img = img.clone()

        noise = self.generateMaskTensor()

        masked_img = torch.ones(img.shape) * (1.0 - noise) + masked_img * noise
        masked_part = torch.ones(img.shape) * noise

        return masked_img, masked_part

    # -----------------------------------------------------------------------------

    def __getitem__(self, index):
        img = Image.open(self.files[index % len(self.files)])
        img = self.transform(img)

        if self.mode == 'train':
            # For training data perform random mask
            masked_img, aux = self.apply_random_mask(img)
        else:
            # For test data mask the center of the image
            masked_img, aux = self.apply_panorama_mask(img)

        return img, masked_img, aux

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
patch_h, patch_w = int(opt.img_size_h / 2**3), int(opt.img_size_w / 2**3)
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
# Adversarial ground truths
# -----------------------------------------------------------------------------
valid = Variable(Tensor(np.ones(patch)), requires_grad=False)
fake = Variable(Tensor(np.zeros(patch)), requires_grad=False)

# -----------------------------------------------------------------------------
# Functions
# -----------------------------------------------------------------------------
def save_sample(batches_done, _Path):
    samples, masked_samples, masked_part = next(iter(test_dataloader))
    samples = Variable(samples.type(Tensor))
    masked_samples = Variable(masked_samples.type(Tensor))
    masked_part = Variable(masked_part.type(Tensor))

    # -----------------------------------------------------------------------------
    # Generate inpainted image
    # -----------------------------------------------------------------------------
    gen_mask = generator(masked_samples)

    # -----------------------------------------------------------------------------
    # Mixture
    # -----------------------------------------------------------------------------
    filled_example = gen_mask * (1 - masked_part) + samples * masked_part

    # -----------------------------------------------------------------------------
    # Transfer mask to image space for saving (-1.0 .. +1.0)
    # -----------------------------------------------------------------------------
    masked_part = (masked_part - 0.5) * 2.0

    # -----------------------------------------------------------------------------
    # Save sample
    # -----------------------------------------------------------------------------
    sample = torch.cat((samples.data, masked_part.data, masked_samples.data, gen_mask.data, filled_example.data, samples.data), -2)
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

            discriminator.train()
            
            # -----------------------------------------------------------------------------

            Checkpoint = LoadCheckpoint(opt.path_to_savepoint + '/model_best_generator.pth.tar')

            LastEpoch            = Checkpoint['epoch']
            GeneratorMinimalLoss = Checkpoint['best_prec1']

            generator.load_state_dict(Checkpoint['state_dict'])
            optimizer_G.load_state_dict(Checkpoint['optimizer'])

            generator.train()

            print ("Loaded extisting checkpoint to resume in epoch", LastEpoch)
        else:
            LastEpoch = 0
            GeneratorMinimalLoss = 1000

        # -----------------------------------------------------------------------------

        for epoch in range(LastEpoch, opt.n_epochs):
            for i, (imgs, masked_imgs, masked_parts) in enumerate(train_dataloader):

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