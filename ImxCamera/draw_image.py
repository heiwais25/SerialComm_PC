def multiply(a,b):
    print("Will compute", a, "times", b)
    c = 0
    for i in range(0, a):
        c = c + b
    return c

def print_array(a):
    print(a)
    print(len(a))
    return 1

def plot_image(raw_image_data):
    print("Hello")
    print(type(raw_image_data))
    
    import numpy as np
    raw_image_data = raw_image_data.astype(int) # Change data type to int
    raw_image_value = np.right_shift(raw_image_data[1::2],4) + np.left_shift(raw_image_data[0::2],4)
    raw_image_value = raw_image_value[::-1]

    # reshape
    raw_image_width = 1319
    raw_image_height = 976
    raw_image_value = np.reshape(raw_image_value, (raw_image_height, raw_image_width))

    # matplotlib
    import matplotlib.pyplot as plt
    fig, ax = plt.subplots()
    cax = plt.imshow(raw_image_value, cmap='gray')
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

    plt.show()

    return 1
