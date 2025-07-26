# xbgs
xcb-based background setter for x

# compile
$ clang xbgs.c -o xbgs -Os -s -Ilib/ -lxcb -lm

# modes
```
  -s      image fits appropriately to the screen size
  -f      adjusts the picture to full view by cropping the screen
  -c      centers the image in the center of the screen
  -z      enlarges if the image is small
```

# usage
$ ./xbgs -s ~/image.jpeg

$ ./xbgs -c ~/image.png

# example image
![image](https://github.com/user-attachments/assets/56591b44-5910-40b8-894b-5f92a06dc5a0)
