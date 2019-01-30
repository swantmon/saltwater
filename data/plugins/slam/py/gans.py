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

        # -----------------------------------------------------------------------------

        self.pool = nn.MaxPool2d(2, 2)

        self.conv1_1 = nn.Conv2d(channels, 64, 3, stride=1, padding=1)

        self.conv2_1 = nn.Conv2d(64, 64, 3, stride=1, padding=1)
        self.norm2_1 = nn.BatchNorm2d(64, 0.8)

        self.conv3_1 = nn.Conv2d(64, 128, 3, stride=1, padding=1)
        self.norm3_1 = nn.BatchNorm2d(128, 0.8)

        self.conv4_1 = nn.Conv2d(128, 256, 3, stride=1, padding=1)
        self.norm4_1 = nn.BatchNorm2d(256, 0.8)

        self.conv5_1 = nn.Conv2d(256, 512, 3, stride=1, padding=1)
        self.norm5_1 = nn.BatchNorm2d(512, 0.8)

        self.conv6_1 = nn.Conv2d(512, 1000, 4)
        self.linear6_2 = nn.Linear(1000, 2048)
        self.drop6_3 = nn.Dropout(0.5)
        self.linear6_4 = nn.Linear(2048, 1000)
        self.drop6_5 = nn.Dropout(0.5)

        self.tconv6_1 = nn.ConvTranspose2d(1000, 512, 4, stride=1, padding=0)
        self.norm6_1 = nn.BatchNorm2d(512, 0.8)

        self.tconv7_1 = nn.ConvTranspose2d(512, 256, 4, stride=2, padding=1)
        self.norm7_1 = nn.BatchNorm2d(256, 0.8)

        self.tconv8_1 = nn.ConvTranspose2d(256, 128, 4, stride=2, padding=1)
        self.norm8_1 = nn.BatchNorm2d(128, 0.8)

        self.tconv9_1 = nn.ConvTranspose2d(128, 64, 4, stride=2, padding=1)
        self.norm9_1 = nn.BatchNorm2d(64, 0.8)

        self.tconv10_1 = nn.ConvTranspose2d(64, 64, 4, stride=2, padding=1)
        self.norm10_1 = nn.BatchNorm2d(64, 0.8)

        self.conv11 = nn.Conv2d(64, channels, 3, 1, 1)

    # -----------------------------------------------------------------------------

    def forward(self, x):
        x = self.conv1_1(x)
        x = self.pool(x)

        x = self.conv2_1(x)
        x = self.norm2_1(x)
        x = self.pool(x)
        x = F.elu(x)

        x = self.conv3_1(x)
        x = self.norm3_1(x)
        x = self.pool(x)
        x = F.elu(x)

        x = self.conv4_1(x)
        x = self.norm4_1(x)
        x = self.pool(x)
        x = F.elu(x)

        x = self.conv5_1(x)
        x = self.norm5_1(x)
        x = self.pool(x)
        x = F.elu(x)

        x = self.conv6_1(x) 

        (_, C, H, W) = x.data.size()
        x = x.view(-1 , C * H * W)

        x = self.linear6_2(x)
        x = self.drop6_3(x)  
        x = self.linear6_4(x)
        x = self.drop6_5(x)   

        x = x.view(-1 , C, H, W)

        x = self.tconv6_1(x)
        x = self.norm6_1(x)
        x = F.elu(x)

        x = self.tconv7_1(x)
        x = self.norm7_1(x)
        x = F.elu(x)

        x = self.tconv8_1(x)
        x = self.norm8_1(x)
        x = F.elu(x)

        x = self.tconv9_1(x)
        x = self.norm9_1(x)
        x = F.elu(x)

        x = self.tconv10_1(x)
        x = self.norm10_1(x)
        x = F.elu(x)

        x = self.conv11(x)
        x = F.tanh(x)

        return x

# -----------------------------------------------------------------------------

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