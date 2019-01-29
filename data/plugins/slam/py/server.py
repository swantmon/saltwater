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
parser.add_argument('--output', type=str, default='./output/', help='output folder of the results')
parser.add_argument('--img_size_w', type=int, default=256, help='width of image dimension')
parser.add_argument('--img_size_h', type=int, default=128, help='height of each image dimension')
parser.add_argument('--path_to_generator', type=str, default='./savepoint/model_best_generator.pth.tar', help='path to saved generator')
parser.add_argument('--port', type=int, default=12346, help='Port address to an endpoint')
opt = parser.parse_args()

# -----------------------------------------------------------------------------
# Output
# -----------------------------------------------------------------------------
os.makedirs(opt.output, exist_ok=True)

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

Tensor = torch.cuda.FloatTensor if cuda else torch.FloatTensor

generator = Generator()

if cuda:
    generator.cuda()

# -----------------------------------------------------------------------------
# Functionality
# -----------------------------------------------------------------------------
def OnNewClient(_Socket, _Address, _ID):
    print ("Accepted connection from client", _Address)

    # -----------------------------------------------------------------------------
    # Prepare for output
    # -----------------------------------------------------------------------------
    os.makedirs('{}{}'.format(opt.output, _Address[0]), exist_ok=True)
    os.makedirs('./tmp/{}'.format(_Address[0]), exist_ok=True)

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
        panorama = panorama / 255.0
        
        masked_sample = Image.new("RGB", (opt.img_size_w, opt.img_size_h), "white")
        masked_sample_pixels = masked_sample.load()

        for y in range(opt.img_size_h):
            for x in range(opt.img_size_w):
                if panorama[y][x][3] != 0.0:
                    masked_sample_pixels[x, y] = (int(panorama[y][x][0] * 255), int(panorama[y][x][1] * 255), int(panorama[y][x][2] * 255))

        transform = transforms.Compose(transforms_)

        masked_sample = transform(masked_sample)

        masked_samples = torch.zeros([1, 3, opt.img_size_h, opt.img_size_w])

        masked_samples[0] = masked_sample

        masked_samples = Variable(masked_samples.type(Tensor))

        gen_mask = generator(masked_samples)

        # -----------------------------------------------------------------------------
        # Send generated image back
        # Now: Save image to see quality
        # TODO: Do not save image! Use data directly.
        # -----------------------------------------------------------------------------
        os.makedirs('./tmp/{}/{}'.format(_Address[0], _ID), exist_ok=True)

        save_image(gen_mask.data, './tmp/{}/{}/tmp_output_generator.png'.format(_Address[0], _ID), nrow=1, normalize=True)  

        im = Image.open('./tmp/{}/{}/tmp_output_generator.png'.format(_Address[0], _ID)).convert('RGBA')

        resultData =  np.asarray(list(im.getdata()))

        resultData = resultData.astype(np.uint8)

        pixels = resultData.tobytes()

        _Socket.sendall(struct.pack('iii', 0, opt.img_size_w * opt.img_size_h * 4, opt.img_size_w * opt.img_size_h * 4))
        _Socket.sendall(pixels)

        # -----------------------------------------------------------------------------
        # Save output and input to file system
        # -----------------------------------------------------------------------------
        sample = torch.cat((masked_samples.data, gen_mask.data), -2)

        os.makedirs('{}{}/{}'.format(opt.output, _Address[0], _ID), exist_ok=True)
        save_image(sample, '{}{}/{}/result_panorama_{}.png'.format(opt.output, _Address[0], _ID, Interval), nrow=1, normalize=True)

        Interval = Interval + 1

    print ("Disconnected from client", _Address)

    if os.path.isfile('./tmp/{}/{}/tmp_output_generator.png'.format(_Address[0], _ID)) == True:
        os.remove('./tmp/{}/{}/tmp_output_generator.png'.format(_Address[0], _ID))
    
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

        generator.load_state_dict(Checkpoint['state_dict'])

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