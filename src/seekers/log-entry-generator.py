import pybookwyrm as bw
import time

def find(wanted, bookwyrm):
    i = 0
    while not bookwyrm.terminating():
        bookwyrm.log(bw.loglevel.info, "log entry from " + __file__ + " " + str(i))
        i += 1
        time.sleep(0.5)
