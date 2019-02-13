import argparse
import glob
import os
import numpy as np
import math
import socket
import struct
import _thread 
import sys

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
parser.add_argument('--output', type=str, default='D:/NN/plugin_slam/output_mrf/', help='output folder of the results')
parser.add_argument('--path_to_dataset', type=str, default='D:/NN/dataset/ILSVRC2012_img_test/', help='path to the dataset (no recursive search)')
parser.add_argument('--img_size_w', type=int, default=128, help='width of image dimension')
parser.add_argument('--img_size_h', type=int, default=128, help='height of each image dimension')
parser.add_argument('--mask_size', type=int, default=64, help='size of random mask')
parser.add_argument('--path_to_generator', type=str, default='D:/NN/plugin_slam/savepoint/model_best_generator.pth.tar', help='path to saved generator')
opt = parser.parse_args()

# -----------------------------------------------------------------------------
# Output
# -----------------------------------------------------------------------------
os.makedirs(opt.output, exist_ok=True)

# -----------------------------------------------------------------------------
# Dataset
# -----------------------------------------------------------------------------
class ImageDataset(Dataset):
    def __init__(self, root, transforms_=None):
        self.transform = transforms.Compose(transforms_)
        self.files = sorted(glob.glob('%s/*.*' % root))

    # -----------------------------------------------------------------------------

    def apply_test_mask(self, img):
        masked_img = img.clone()
        
        y1 = int((opt.img_size_h - opt.mask_size) / 2)
        x1 = int((opt.img_size_w - opt.mask_size) / 2)

        y2, x2 = y1 + opt.mask_size, x1 + opt.mask_size

        masked_img[:, y1:y2, x1:x2] = 1
        masked_part = img[:, y1:y2, x1:x2]

        return masked_img, masked_part, ([x1, y1, x2, y2])

    # -----------------------------------------------------------------------------

    def __getitem__(self, index):
        img = Image.open(self.files[index % len(self.files)])
        img = img.convert('RGB')
        img = self.transform(img)

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
# GAN & Settings
# -----------------------------------------------------------------------------
transforms_ = [ transforms.Resize((opt.img_size_h, opt.img_size_w), Image.BICUBIC),
                transforms.ToTensor(),
                transforms.Normalize((0.5,0.5,0.5), (0.5,0.5,0.5)) ]

test_dataloader = DataLoader(ImageDataset(opt.path_to_dataset, transforms_=transforms_),
                        batch_size=1, shuffle=True, num_workers=0)

Tensor = torch.cuda.FloatTensor if cuda else torch.FloatTensor

generator = Generator()

if cuda:
    generator.cuda()

# -----------------------------------------------------------------------------
# Main function
# -----------------------------------------------------------------------------
if __name__ == '__main__':
    try:

        # -----------------------------------------------------------------------------
        # Load best model from path
        # -----------------------------------------------------------------------------
        Model = LoadCheckpoint(opt.path_to_generator)

        generator.load_state_dict(Model['state_dict'])

        print ("Loaded model")

        # -----------------------------------------------------------------------------
        # Prepare for output
        # -----------------------------------------------------------------------------
        os.makedirs(opt.output, exist_ok=True)

        # -----------------------------------------------------------------------------
        # MRF creation
        # -----------------------------------------------------------------------------

        # -----------------------------------------------------------------------------
        # Main loop
        # -----------------------------------------------------------------------------
        for _ in range(0, 10):
            for i, (imgs, masked_imgs, masked_parts, position) in enumerate(test_dataloader):
                imgs = Variable(imgs.type(Tensor))
                masked_imgs = Variable(masked_imgs.type(Tensor))

                x1 = position[0][0].item()
                y1 = position[1][0].item()
                x2 = position[2][0].item()
                y2 = position[3][0].item()

                # -----------------------------------------------------------------------------
                # Generate inpainted image
                # -----------------------------------------------------------------------------
                gen_mask = generator(masked_imgs)

                filled_sample = masked_imgs.clone()
                filled_sample[:, :, y1:y2, x1:x2] = gen_mask

                # -----------------------------------------------------------------------------
                # MRF
                # -----------------------------------------------------------------------------


                # -----------------------------------------------------------------------------
                # Save output and input to file system
                # -----------------------------------------------------------------------------
                sample = torch.cat((imgs.data, filled_sample.data), -2)

                save_image(sample, '{}/result.png'.format(opt.output, ), nrow=1, normalize=True)
        
        print ("Finished")

    except OSError as _Error:
        print("OS error: {0}".format(_Error))
    except ValueError:
        print("Could not convert data to an value.")
    except:
        print("Unexpected error:", sys.exc_info()[0])
        raise