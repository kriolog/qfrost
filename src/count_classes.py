#!/usr/bin/python2
import os
import re

def main():
    classes = set()

    for root, folders, files in os.walk("."):
        for file in files:
            name, ext = os.path.splitext(file)

            if ext.lower() not in [".h", ".hpp", ".hxx"]:
                continue

            f = open(os.path.join(root, file))
            for l in f:
                m = re.match(r'class ([a-zA-Z0-9]*)[^;]*$', l)
                if not m:
                    continue

                classes.add(m.groups())

            f.close()

    print len(classes)

if __name__ == "__main__":
    main()
