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
from PIL import ImageOps

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
parser.add_argument('--output', type=str, default=None, help='output folder of the results')
parser.add_argument('--img_size_w', type=int, default=256, help='width of image dimension')
parser.add_argument('--img_size_h', type=int, default=128, help='height of each image dimension')
parser.add_argument('--path_to_generator', type=str, default='./savepoint/model_best_generator.pth.tar', help='path to saved generator')
parser.add_argument('--port', type=int, default=12346, help='Port address to an endpoint')
parser.add_argument('--flip', type=bool, default=False, help='flip input image')
opt, unknown_opt = parser.parse_known_args()

# -----------------------------------------------------------------------------
# Check if cuda is available
# -----------------------------------------------------------------------------
cuda = torch.cuda.is_available()

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
        for i, m, s in zip(range(tensor.size(0)), self.mean, self.std):
            t = tensor[i]
            t.mul_(s).add_(m)
            # The normalize code -> t.sub_(m).div_(s)
        return tensor

transforms_ = [ transforms.Resize((opt.img_size_h, opt.img_size_w), Image.BICUBIC),
                transforms.ToTensor(),
                transforms.Normalize((0.5, 0.5, 0.5), (0.5, 0.5, 0.5)) ]
transforms__ = [ transforms.Normalize((0, 0 , 0), (1.0 / 0.5, 1.0 / 0.5, 1.0 / 0.5)),
                 transforms.Normalize((-0.5, -0.5, -0.5), (1.0, 1.0, 1.0)),
                 transforms.ToPILImage(mode='RGB'),
                 transforms.Resize((opt.img_size_h, opt.img_size_w), Image.BICUBIC) ]

Tensor = torch.cuda.FloatTensor if cuda else torch.FloatTensor

generator = Generator()

if cuda:
    generator.cuda()

# -----------------------------------------------------------------------------
# Functionality
# -----------------------------------------------------------------------------
transform1 = transforms.Compose(transforms_)
transform2 = transforms.Compose(transforms__)

def OnNewClient(_Socket, _Address, _ID):
    print ("Accepted connection from client", _Address)

    # -----------------------------------------------------------------------------
    # Prepare for output
    # -----------------------------------------------------------------------------
    os.makedirs('{}{}'.format(opt.output, _Address[0]), exist_ok=True)

    # -----------------------------------------------------------------------------
    # Wait for data
    # -----------------------------------------------------------------------------
    IsRunning = True
    Interval = 0
    
    while (IsRunning):
        print(_Address, "Wait for data...")

        try:
            header = _Socket.recv(12)

            integers = struct.unpack('I' * 3, header)
            bytesLeft = integers[1]

            textureSize = _Socket.recv(8)
            bytesLeft -= len(textureSize)
            
            panorama = np.array([])

            print(_Address, "Receiving ", bytesLeft, " byte of data")

            while bytesLeft > 0:
                buffer = _Socket.recv(bytesLeft)
                bytesLeft -= len(buffer)

                nparray = np.frombuffer(buffer, dtype=np.uint8)
                panorama = np.concatenate([panorama, nparray])
        except:
            print(_Address, "An error occured during waiting for header payload data")
            IsRunning = False
            continue

        print(_Address, "Data received")

        # -----------------------------------------------------------------------------
        # Use generator to create estimation
        # -----------------------------------------------------------------------------
        panorama.shape = (opt.img_size_h, opt.img_size_w, 4)

        if opt.flip == True: panorama = np.flip(panorama, 0)

        OriginalImage = Image.fromarray(np.uint8(panorama), "RGBA")

        OriginalMask  = OriginalImage.split()[-1]

        panorama /= 255

        panorama[:,:,3][panorama[:,:,3] <  0.5] = 0.0
        panorama[:,:,3][panorama[:,:,3] >= 0.5] = 1.0

        panorama[:,:,0] = panorama[:,:,0] * panorama[:,:,3] + 1.0 * (1.0 - panorama[:,:,3])
        panorama[:,:,1] = panorama[:,:,1] * panorama[:,:,3] + 1.0 * (1.0 - panorama[:,:,3])
        panorama[:,:,2] = panorama[:,:,2] * panorama[:,:,3] + 1.0 * (1.0 - panorama[:,:,3])

        panorama *= 255
        
        InputImage = Image.fromarray(np.uint8(panorama), "RGBA")

        InputMask = InputImage.split()[-1]

        masked_sample = InputImage.convert("RGB")

        masked_sample = transform1(masked_sample)

        masked_samples = torch.zeros([1, 3, opt.img_size_h, opt.img_size_w])

        masked_samples[0] = masked_sample

        masked_samples = Variable(masked_samples.type(Tensor))

        gen_masks = generator(masked_samples)

        # -----------------------------------------------------------------------------
        # Send generated image back
        # -----------------------------------------------------------------------------
        gen_mask = gen_masks[0]

        if cuda: gen_mask = gen_mask.cpu()       

        OutputImage = transform2(gen_mask)

        OutputImage = OutputImage.convert('RGBA')

        if opt.flip == True: OutputImage = ImageOps.flip(OutputImage)

        resultData =  np.asarray(list(OutputImage.getdata()))

        resultData = resultData.astype(np.uint8)

        pixels = resultData.tobytes()

        _Socket.sendall(struct.pack('iii', 0, opt.img_size_w * opt.img_size_h * 4 + 2 * 4, opt.img_size_w * opt.img_size_h * 4 + 2 * 4))
        _Socket.sendall(struct.pack('ii', opt.img_size_w, opt.img_size_h))
        _Socket.sendall(pixels)

        if opt.flip == True: OutputImage = ImageOps.flip(OutputImage)

        # -----------------------------------------------------------------------------
        # Save output and input to file system
        # -----------------------------------------------------------------------------

        if opt.output is not None:
            os.makedirs(opt.output, exist_ok=True)
            
            Result = Image.new('RGBA', (256, 768))

            Result.paste(OriginalImage, (0, 0))
            Result.paste(OriginalMask, (0, 128))
            Result.paste(InputImage.convert('RGB'), (0, 256))
            Result.paste(InputMask, (0, 384))
            Result.paste(OutputImage, (0, 512))
            Result.paste(Image.alpha_composite(OutputImage, OriginalImage), (0, 640))

            os.makedirs('{}{}/{}'.format(opt.output, _Address[0], _ID), exist_ok=True)
            Result.save('{}{}/{}/result_panorama_{}.png'.format(opt.output, _Address[0], _ID, Interval))

        Interval = Interval + 1

    print ("Disconnected from client", _Address)
    
    _Socket.close()

# -----------------------------------------------------------------------------
# Main function
# -----------------------------------------------------------------------------
if __name__ == '__main__':
    try:

        # -----------------------------------------------------------------------------
        # Load best model from path
        # -----------------------------------------------------------------------------
        Checkpoint = LoadCheckpoint(opt.path_to_generator)

        generator.load_state_dict(Checkpoint['state_dict'], cuda)

        generator.eval()

        print ("Loaded extisting checkpoint")

        # -----------------------------------------------------------------------------
        # Open socket and connect or listen
        # -----------------------------------------------------------------------------          
        Socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        print ("Start server on port %d" % opt.port)

        Socket.bind(('', opt.port))
        Socket.listen()

        SocketID = 0

        while True:
            print ("Wait for client...")

            Client, Address = Socket.accept()

            _thread.start_new_thread(OnNewClient, (Client, Address, SocketID))

            SocketID = SocketID + 1
        
        Socket.close()

    except OSError as _Error:
        print("OS error: {0}".format(_Error))
    except ValueError:
        print("Could not convert data to an value.")
    except:
        print("Unexpected error:", sys.exc_info()[0])
        raise