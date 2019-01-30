
import argparse
import os
import requests


parser = argparse.ArgumentParser()
parser.add_argument('--path_to_txt', type=str, default="data/fall11_urls.txt", help='Path to txt file w/ URLs')
parser.add_argument('--output_path', type=str, default="data/IMAGE_NET/", help='Path to store output')
opt = parser.parse_args()

os.makedirs(opt.output_path, exist_ok=True)

with open(opt.path_to_txt) as f:
    for line in f:
        (pic_file, pic_url) = line.split('\t')

        pic_url = pic_url.replace('\n', '')

        pic_url_file, pic_url_ext = os.path.splitext(pic_url)

        pic_file = opt.output_path + pic_file + pic_url_ext

        try:
            response = requests.get(pic_url, timeout=4)
            if response.status_code == requests.codes.OK:
                with open(pic_file, 'wb') as handler:
                    handler.write(response.content)
                    print ("Downloaded: " + pic_url)    
            else:
                print ("No response: " + pic_url)    
        except:
            print ("Invalid: " + pic_url)