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
binimg encode container.png somefile.mp4
binimg decode encoded.png result.mp4
binimg decode examples/decode-me.png not-a-rickroll.mp4
binimg decode examples/decode-me.png safepic.mp4
```