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
parser.add_argument('--data', type=str, default='/Users/tschwandt/Documents/Archive/data/', help='output folder of the results')
parser.add_argument('--path_to_generator', type=str, default='/Users/tschwandt/Documents/Archive/model_best_generator.pth.tar', help='path to saved generator')
opt = parser.parse_args()

# -----------------------------------------------------------------------------
# Check if cuda is available
# -----------------------------------------------------------------------------
cuda = True if torch.cuda.is_available() else False

# -----------------------------------------------------------------------------
# GAN & Settings
# -----------------------------------------------------------------------------
class Denormalize(object):
    def __init__(self, mean, std):
        self.mean = mean
        self.std = std

    def __call__(self, tensor):
        """
        Args:
            tensor (Tensor): Tensor image of size (C, H, W) to be normalized.
        Returns:
            Tensor: Normalized image.
        """
        for t, m, s in zip(tensor, self.mean, self.std):
            t.mul_(s).add_(m)
            # The normalize code -> t.sub_(m).div_(s)
        return tensor

transforms_ = [ transforms.Resize((128, 256), Image.BICUBIC),
                transforms.ToTensor(),
                transforms.Normalize((0.5,0.5,0.5), (0.5,0.5,0.5)) ]

transforms__ = [ Denormalize((0.5,0.5,0.5), (0.5,0.5,0.5)),
                 transforms.ToPILImage(mode='RGB'),
                 transforms.Resize((128, 256), Image.BICUBIC) ]

Tensor = torch.cuda.FloatTensor if cuda else torch.FloatTensor

generator = Generator()

if cuda:
    generator.cuda()

transform1 = transforms.Compose(transforms_)
transform2 = transforms.Compose(transforms__)

def EstimateEnvironment(_Panorama):
    masked_sample = transform1(_Panorama)

    masked_samples = torch.zeros([1, 3, 128, 256])

    masked_samples[0] = masked_sample

    masked_samples = Variable(masked_samples.type(Tensor))

    gen_mask = generator(masked_samples)

    gen_mask = gen_mask[0]

    return transform2(gen_mask)

# -----------------------------------------------------------------------------
# Main function
# -----------------------------------------------------------------------------
if __name__ == '__main__':
    try:
        # -----------------------------------------------------------------------------
        # Load best model from path
        # -----------------------------------------------------------------------------
        Checkpoint = LoadCheckpoint(opt.path_to_generator)

        generator.load_state_dict(Checkpoint['state_dict'])

        print ("Loaded extisting checkpoint")

        ListOfImages = []

        for root, dirs, files in os.walk(opt.data):
            for file in files:
                if file.endswith(".png"):
                    ListOfImages.append(os.path.join(root, file))

        for _ImageFilepath in ListOfImages:
            print(_ImageFilepath)

            Img = Image.open(_ImageFilepath)

            Pano = Img.crop((0, 0, 256, 128))

            Estimation = EstimateEnvironment(Pano)

            Result = Image.new('RGB', Img.size)

            Result.paste(Img, (0, 0))
            Result.paste(Estimation, (0, 128))

            Result.save(_ImageFilepath)


    except OSError as _Error:
        print("OS error: {0}".format(_Error))
    except ValueError:
        print("Could not convert data to an value.")
    except:
        print("Unexpected error:", sys.exc_info()[0])
        raise