# xbgs
xcb-based background setter for x

# compile
$ clang xbgs.c -o xbgs -Ilib/ -lxcb -lm

# modes
```
  --bg-scale    image fits appropriately to the screen size
  --bg-fill     adjusts the picture to full view by cropping the screen
  --bg-center   centers the image in the center of the screen
  --bg-zoom     enlarges if the image is small
```

# usage
$ ./xbgs --bg-scale ~/image.jpeg

# example image
![image](https://github.com/user-attachments/assets/719a51a4-6c3f-4f00-8cfb-22c1a26d19d6)
