# BigPicCreator
Console utility for creating *.jpeg pictures of unreasonably big sizes 

## How to build
Clone this project
In /src folder 
```bash
make build
```
Or
```bash
make debug
```

## How to use
You can use short or full optarg names
```bash
./build/bin/main --file=jopa --type=png --size=100m
```

```bash
./build/bin/main -f jopa -t png -s 100m
```
For size suffixes you can use byte kilobyte megabyte etc suffixes like 1b 1k 1m
