import sys

data = sys.stdin.read()

print("Python got: ", data)
print("errors from python", file=sys.stderr)
