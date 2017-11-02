import numpy as np
import matplotlib.pyplot as plt
import time

raw_image_width = 1319
raw_image_height = 976    

def plot_image(raw_image_data):
    raw_image_nparray = image_processing_1d_to_2d(raw_image_data, raw_image_width, raw_image_height)
    fig = set_plotting_detail(raw_image_nparray)    
    file_name = 'output/%s.png' % (getCurrentDayHourMinuteSec())
    fig.savefig(file_name)
    print("Image saved at '%s'" % file_name)
    plt.show()
    return 1


def save_image_by_numpy(raw_image_data):
    raw_image_nparray = image_processing_1d_to_2d(raw_image_data, raw_image_width, raw_image_height)
    file_name = 'output/%s_np2dArray' % (getCurrentDayHourMinuteSec())
    raw_image_nparray.tofile(file_name)
    print("File saved at '%s'" % file_name)
    return 1



# Set detail information of plotting in this function
def set_plotting_detail(raw_image_nparray):
    fig, ax = plt.subplots()
    cax = plt.imshow(raw_image_nparray, cmap='gray')
    fig.subplots_adjust(right=0.97, left=0.15)
    cbar = fig.colorbar(cax)
    cbar.ax.tick_params(labelsize=13)
    fig.set_size_inches(12,8)
    plt.clim(0,4096)
    plt.gca().invert_yaxis()


    plt.xticks(fontsize=12)
    plt.yticks(fontsize=12)
    plt.title('CCD Camera raw image', fontsize=15)
    plt.xlabel('horizontal pixel', fontsize=15, labelpad=20)
    plt.ylabel('vertical pixel', fontsize=15, labelpad=20)    
    return fig

# It is necessary to draw np 2d array
def image_processing_1d_to_2d(raw_image_data, width, height):
    raw_image_data_int = raw_image_data.astype(np.int16) # Change data type to int
    raw_image_nparray = np.right_shift(raw_image_data_int[1::2],4) + np.left_shift(raw_image_data_int[0::2],4)
    raw_image_nparray = raw_image_nparray[::-1]

    # reshape
    raw_image_nparray = np.reshape(raw_image_nparray, (height, width))
    print("Dimension of image is %d x %d" % (width, height))
    return raw_image_nparray


# ----------------------------------------------------------------------------------------------
# 
#  Helper function
# 
# ----------------------------------------------------------------------------------------------
def CheckWriteReadFile(file_name, write_array):
    raw_data = np.fromfile(file_name, dtype=np.int16)
    raw_data = np.reshape(raw_data, (raw_image_height, raw_image_width))
    return raw_data == write_array

def getCurrentDayHourMinuteSec():
    time_info = "%02d_%02d_%02d_%02d" %(time.gmtime().tm_mday, time.gmtime().tm_hour, 
        time.gmtime().tm_min, time.gmtime().tm_sec)
    return time_info