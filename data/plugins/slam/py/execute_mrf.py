import argparse
import glob
import os
import numpy as np
import math
import socket
import struct
import _thread 
import sys

import cv2

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

from extern.neural_style import *

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

# Basic options
parser.add_argument("-style_blend_weights", default=None) 
parser.add_argument("-image_size", help="Maximum height / width of generated image", type=int, default=384)
parser.add_argument("-gpu", help="Zero-indexed ID of the GPU to use; for CPU mode set -gpu = -1", type=int, default=0)

# Optimization options
#parser.add_argument("-content_weight", type=float, default=5e0) 
parser.add_argument("-content_weight", type=float, default=8e1) 
#parser.add_argument("-style_weight", type=float, default=1e2)
parser.add_argument("-style_weight", type=float, default=6e2)
#parser.add_argument("-tv_weight", type=float, default=1e-3)
parser.add_argument("-tv_weight", type=float, default=0)
parser.add_argument("-num_iterations", type=int, default=1000)
#parser.add_argument("-init", choices=['random', 'image'], default='random')
parser.add_argument("-init", choices=['random', 'image'], default='image')
parser.add_argument("-init_image", default=None)
parser.add_argument("-optimizer", choices=['lbfgs', 'adam'], default='lbfgs')
parser.add_argument("-learning_rate", type=float, default=1e0)
parser.add_argument("-lbfgs_num_correction", type=int, default=0)

# Output options      
parser.add_argument("-print_iter", type=int, default=50)
parser.add_argument("-save_iter", type=int, default=100)
parser.add_argument("-output_image", default='intermediate.png')

# Other options
parser.add_argument("-style_scale", type=float, default=1.0)
parser.add_argument("-original_colors", type=int, choices=[0, 1], default=0)
parser.add_argument("-pooling", choices=['avg', 'max'], default='max')
parser.add_argument("-model_file", type=str, default='D:/NN/plugin_slam/models/vgg19-d01eb7cb.pth')
parser.add_argument("-backend", choices=['nn', 'cudnn', 'mkl'], default='nn')
parser.add_argument("-cudnn_autotune", action='store_true')
parser.add_argument("-seed", type=int, default=-1)

parser.add_argument("-content_layers", help="layers for content", default='relu4_2')
#parser.add_argument("-style_layers", help="layers for style", default='relu1_1,relu2_1,relu3_1,relu4_1,relu5_1')
parser.add_argument("-style_layers", help="layers for style", default='relu3_1,relu4_1')

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
                        batch_size=1, shuffle=False, num_workers=0)

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

            os.makedirs('{}/{}'.format(opt.output, i), exist_ok=True)

            save_image(imgs, '{}/{}/0_original.png'.format(opt.output, i), nrow=1, normalize=True)

            save_image(masked_imgs, '{}/{}/1_masked.png'.format(opt.output, i), nrow=1, normalize=True)

            save_image(filled_sample, '{}/{}/2_prediction_1.png'.format(opt.output, i), nrow=1, normalize=True)

            save_image(gen_mask, '{}/{}/3_cutout_1.png'.format(opt.output, i), nrow=1, normalize=True)      

            mask = torch.zeros([128, 128])

            mask[y1:y2, x1:x2] = torch.ones([64, 64])

            save_image(mask, '{}/{}/X_mask.png'.format(opt.output, i), nrow=1, normalize=True)      

            style_image = filled_sample.clone()

            #style_image[:, :, 32:63 , 32:96] = masked_imgs[:, :, 1 :32 , 32:96]
            #style_image[:, :, 63:77 , 32:96] = masked_imgs[:, :, 17:31 , 32:96]
            #style_image[:, :, 78:109, 32:96] = masked_imgs[:, :, 97:128, 32:96]

            #style_image[:, :, 117:232, 117:396] = style_image[:, :, 1  :116, 117:396]
            #style_image[:, :, 233:280, 117:396] = style_image[:, :, 69 :116, 117:396]
            #style_image[:, :, 281:396, 117:396] = style_image[:, :, 397:512, 117:396]      

            save_image(style_image, '{}/{}/4_style.png'.format(opt.output, i), nrow=1, normalize=True)      

            # -----------------------------------------------------------------------------
            # OpenCV Inpainting
            # -----------------------------------------------------------------------------
            img  = cv2.imread('{}/{}/1_masked.png'.format(opt.output, i))
            mask = cv2.imread('{}/{}/X_mask.png'.format(opt.output, i),0)

            dst = cv2.inpaint(img, mask, 3, cv2.INPAINT_TELEA)

            cv2.imwrite('{}/{}/2_prediction_2.png'.format(opt.output, i), dst)

            # -----------------------------------------------------------------------------
            # MRF
            # -----------------------------------------------------------------------------
            Result = NeuralStyle(opt, '{}/{}/3_cutout_1.png'.format(opt.output, i), '{}/{}/4_style.png'.format(opt.output, i), _InitImage = '{}/{}/3_cutout_1.png'.format(opt.output, i))

            ResizeResult = transforms.Compose([
                transforms.ToPILImage(),
                transforms.Resize((opt.mask_size, opt.mask_size), Image.BICUBIC),
                transforms.ToTensor(),
            ])

            resized_result = ResizeResult(Result * 0.5 + 0.5) * 2.0 - 1.0

            filled_sample[:, :, y1:y2, x1:x2] = resized_result

            # -----------------------------------------------------------------------------
            # Save output and input to file system
            # -----------------------------------------------------------------------------
            save_image(filled_sample, '{}/{}/2_prediction_3.png'.format(opt.output, i), nrow=1, normalize=True)

            save_image(resized_result, '{}/{}/3_cutout_3.png'.format(opt.output, i), nrow=1, normalize=True)
        
        print ("Finished")

    except OSError as _Error:
        print("OS error: {0}".format(_Error))
    except ValueError:
        print("Could not convert data to an value.")
    except:
        print("Unexpected error:", sys.exc_info()[0])
        raise