# xbgs
xcb-based background setter for x

# compile
$ clang xbgs.c -o xbgs -Os -s -Ilib/ -lxcb -lm

# modes
```
  -s        image fits appropriately to the screen size

  -f        adjusts the picture to full view by cropping the screen

  -c        centers the image in the center of the screen

  -z        enlarges if the image is small
```

# usage
$ ./xbgs -s ~/images/image.jpeg

# example image
![image](https://github.com/user-attachments/assets/719a51a4-6c3f-4f00-8cfb-22c1a26d19d6)
