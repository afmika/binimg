# binimg
Hide any binary file inside a picture !

# Building
```
git clone --recursive https://github.com/afmika/binimg.git
cd binimg
premake5 gmake
make
```

# Usage
```bash
binimg encode container input
binimg decode container [output]
```
# Examples
```bash
binimg encode container.png somefile.mp4
binimg decode encoded.png result.mp4
binimg decode examples/decode-me.png
binimg decode examples/decode-me.png not-a-rickroll.mp4
```