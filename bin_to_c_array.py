
import sys

def bytes_from_file(filename, chunksize=8192):
    with open(filename, "rb") as f:
        while True:
            chunk = f.read(chunksize)
            if chunk:
                for b in chunk:
                    yield b
            else:
                break

def main():
    with open(sys.argv[2], "w") as out:
        out.write("char " + sys.argv[3] + "[] = {")
        for b in bytes_from_file(sys.argv[1]):
            out.write(f"{b},")
        out.write("};");

if __name__ == "__main__":
    main()
