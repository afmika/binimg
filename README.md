# bin-img
Hide any binary file inside a picture !

# Building
```
premake5 gmake
make
```

# Usage
```bash
binimg encode container input
binimg decode container output
```
# Examples
```bash
binimg encode container.png rickroll.mp4
binimg decode encoded.png output.mp4
```