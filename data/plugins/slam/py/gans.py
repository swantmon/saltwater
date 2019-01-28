import torch.nn as nn
import torch.nn.functional as F
import torch


class Generator(nn.Module):
    def __init__(self, channels=3):
        super(Generator, self).__init__()

        def fully_connect(in_feat, out_feat):
            layer = nn.Linear(in_feat, out_feat)
            layers = [layer]
            return layers

        def downsample(in_feat, out_feat, normalize=True):
            layers = [nn.Conv2d(in_feat, out_feat, 4, stride=2, padding=1)]
            if normalize:
                layers.append(nn.BatchNorm2d(out_feat, 0.8))
            layers.append(nn.LeakyReLU(0.2))
            return layers

        def upsample(in_feat, out_feat, normalize=True):
            layers = [nn.ConvTranspose2d(in_feat, out_feat, 4, stride=2, padding=1)]
            if normalize:
                layers.append(nn.BatchNorm2d(out_feat, 0.8))
            layers.append(nn.ReLU())
            return layers

        self.pool = nn.MaxPool2d(2, 2)

        self.conv1 = nn.Conv2d(channels, 64, 4, stride=1, padding=2)

#        self.test1 = nn.ConvTranspose2d(64, 128, 4, stride=2, padding=1)
#        self.test2 = nn.BatchNorm2d(128, 0.8)
#        self.test3 = nn.ConvTranspose2d(128, 256, 4, stride=2, padding=1)
#        self.test4 = nn.BatchNorm2d(256, 0.8)
#        self.test5 = nn.Conv2d(256, 128, 4, stride=1, padding=2)
#        self.test6 = nn.BatchNorm2d(128, 0.8)
#        self.test7 = nn.Conv2d(128, 64, 4, stride=1, padding=2)
#        self.test8 = nn.BatchNorm2d(64, 0.8)

        self.conv2 = nn.Conv2d(64, 64, 4, stride=1, padding=2)
        self.norm2 = nn.BatchNorm2d(64, 0.8)
        self.conv3 = nn.Conv2d(64, 128, 4, stride=1, padding=2)
        self.norm3 = nn.BatchNorm2d(128, 0.8)
        self.conv4 = nn.Conv2d(128, 256, 4, stride=1, padding=2)
        self.norm4 = nn.BatchNorm2d(256, 0.8)
        self.conv5 = nn.Conv2d(256, 512, 4, stride=1, padding=2)
        self.norm5 = nn.BatchNorm2d(512, 0.8)

        self.conv6 = nn.Conv2d(512, 1000, 4)

        self.tconv1 = nn.ConvTranspose2d(1000, 512, 4, stride=1, padding=0)
        self.norm6 = nn.BatchNorm2d(512, 0.8)
        self.tconv2 = nn.ConvTranspose2d(512, 256, 4, stride=2, padding=1)
        self.norm7 = nn.BatchNorm2d(256, 0.8)
        self.tconv3 = nn.ConvTranspose2d(256, 128, 4, stride=2, padding=1)
        self.norm8 = nn.BatchNorm2d(128, 0.8)
        self.tconv4 = nn.ConvTranspose2d(128, 64, 4, stride=2, padding=1)
        self.norm9 = nn.BatchNorm2d(64, 0.8)
        self.tconv5 = nn.ConvTranspose2d(64, 64, 4, stride=2, padding=1)
        self.norm10 = nn.BatchNorm2d(64, 0.8)

        self.conv7 = nn.Conv2d(64, channels, 3, 1, 1)


    def forward(self, x):
        x = self.pool(self.conv1(x))                # 128 x 128 > 64 x 64

#        x = self.test1(x)
#        x = F.relu(self.test2(x))
#        x = self.test3(x)
#        x = F.relu(self.test4(x))
#        x = self.pool(self.test5(x))
#        x = F.leaky_relu(self.test6(x), 0.2)
#        x = self.pool(self.test7(x))
#        x = F.leaky_relu(self.test8(x), 0.2)

        x = self.pool(self.conv2(x))                # 64 x 64 > 32 x 32
        x = F.leaky_relu(self.norm2(x), 0.2)
        x = self.pool(self.conv3(x))                # 32 x 32 > 16 x 16
        x = F.leaky_relu(self.norm3(x), 0.2)
        x = self.pool(self.conv4(x))                # 16 x 16 > 8 x 8
        x = F.leaky_relu(self.norm4(x), 0.2)
        x = self.pool(self.conv5(x))                # 8 x 8 > 4 x 4
        x = F.leaky_relu(self.norm5(x), 0.2)
        x = self.conv6(x)                           # 4 x 4 > 1 x 1
        x = self.tconv1(x)                          # 1 x 1 > 4 x 4
        x = F.relu(self.norm6(x))
        x = self.tconv2(x)
        x = F.relu(self.norm7(x))
        x = self.tconv3(x)
        x = F.relu(self.norm8(x))
        x = self.tconv4(x)
        x = F.relu(self.norm9(x))
        x = self.tconv5(x)
        x = F.relu(self.norm10(x))
        x = F.tanh(self.conv7(x))

        return x

class Discriminator(nn.Module):
    def __init__(self, channels=3):
        super(Discriminator, self).__init__()

        def discriminator_block(in_filters, out_filters, stride, normalize):
            """Returns layers of each discriminator block"""
            layers = [nn.Conv2d(in_filters, out_filters, 3, stride, 1)]
            if normalize:
                layers.append(nn.InstanceNorm2d(out_filters))
            layers.append(nn.LeakyReLU(0.2, inplace=True))
            return layers

        layers = []
        in_filters = channels
        for out_filters, stride, normalize in [ (64, 2, False),
                                                (128, 2, True),
                                                (256, 2, True),
                                                (512, 1, True)]:
            layers.extend(discriminator_block(in_filters, out_filters, stride, normalize))
            in_filters = out_filters

        layers.append(nn.Conv2d(out_filters, 1, 3, 1, 1))

        self.model = nn.Sequential(*layers)

    def forward(self, img):
        return self.model(img)

# -----------------------------------------------------------------------------
# Saving / Loading
# -----------------------------------------------------------------------------
def SaveCheckpoint(_Epoch, _ModelDict, _BestPrecision, _OptimizerDict, _Filename='checkpoint.pth.tar'):
    torch.save({
            'epoch': _Epoch + 1,
            'state_dict': _ModelDict,
            'best_prec1': _BestPrecision,
            'optimizer' : _OptimizerDict,
        }, _Filename)

def LoadCheckpoint(_Filename='checkpoint.pth.tar'):
    return torch.load(_Filename)