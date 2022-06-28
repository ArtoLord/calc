import time


expression = "1+2*(3+4)-1000+1000*12345/123.+0-1234*3456"
count = 1000000

if __name__ == "__main__":
    start = time.time()
    for i in range(count):
        print(expression)