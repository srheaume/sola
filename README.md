# Syncrhonized Overlap-Add (SOLA) Algorithm
The synchronized overlap-add (SOLA) algorithm is an audio time-scale
modification technique. It operates in the time domain and uses a correlation
technique to ensure that synthesis frames overlap in a synchronous manner.

## Usage
```
Usage: sola <source> <destination> <alpha> [<framesize>]
  source       Specifies the file to be time-scale modified
  destination  Specifies the filename for the new file
  alpha        Specifies the time-scale factor [0.5 to 2.0]
  framesize    Specifies the size of the overlapping frames
                 [25 to 1000] {default = 160}
```

# Author
Stephane Rheaume (stephanerheaume@hotmail.com)
