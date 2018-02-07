# Introduction 
- This package is designed for the purpose of serial communication between PC and board through UART to use camera module
- For now available version is only windows. Linux version will be made soon
- The basic communication protocol is UART with specific PACKET protocol to use CRC check

# Environment
- It includes LibPng, LibZ, python, python/numpy header file
- It assumes using Anaconda 64bit as a default python interpreter with matplotlib, numpy
- To resolve environment problem, you need to set environment ahead using this package
<pre><code>QT_PLUGIN_PATH=$(AnacondaDir)/Library/plugins</code></pre>
<pre><code>PYTHONHOME=$(AnacondaDir)</code></pre>
<pre><cpde>PYTHONPATH=$(AnacondaDir)/Lib</code></pre>
- If there are problem with python.dll, it is due to python version problem. You can solve this problem by copying python36.dll file from Anaconda directory

