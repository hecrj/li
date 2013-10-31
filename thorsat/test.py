#!/usr/bin/env python

import os
import sys
import time
import subprocess
import threading

def colorize(string, color, out=sys.stdout):
    colors = {'red': 91 ,'green': 92 ,'yellow': 93, 'blue': 94}
    if out.isatty():
        format = '\033[%dm%%s\033[0m' % colors.get(color, 0)
    else:
        format = '%s'
    out.write(format %  string)
    out.flush()


# Picosat uses the opposite
SATISFIABLE = 20
UNSATISFIABLE = 10

tests = (
    ("100-1",SATISFIABLE, 0.01),
    ("100-2", SATISFIABLE, 0),
    ("100-3", UNSATISFIABLE, 0.01),
    ("100-4", UNSATISFIABLE, 0),
    ("100-5", SATISFIABLE, 0),
    ("100-6", SATISFIABLE, 0.02),
    ("100-7", SATISFIABLE, 0),
    ("100-8", SATISFIABLE, 0),
    ("100-9", UNSATISFIABLE, 0),
    ("100-10", UNSATISFIABLE, 0.01),
    ("150-1", SATISFIABLE, 0.05),
    ("150-2", SATISFIABLE, 0.06),
    ("150-3", SATISFIABLE, 0.04),
    ("150-4", SATISFIABLE, 0.16),
    ("150-5", UNSATISFIABLE, 0.17),
    ("150-6", SATISFIABLE, 0.03),
    ("150-7", UNSATISFIABLE, 0.1),
    ("150-8", SATISFIABLE, 0),
    ("150-9", SATISFIABLE, 0.16),
    ("150-10", UNSATISFIABLE, 0.08),
    ("200-1", UNSATISFIABLE, 1.47),
    ("200-2", SATISFIABLE, 0.03),
    ("200-3", UNSATISFIABLE, 2.2),
    ("200-4", UNSATISFIABLE, 0.62),
    ("200-5", UNSATISFIABLE, 2.72),
    ("200-6", SATISFIABLE, 5.45),
    ("200-7", UNSATISFIABLE, 5.94),
    ("200-8", SATISFIABLE, 0.45),
    ("200-9", SATISFIABLE, 0.63),
    ("200-10", UNSATISFIABLE, 2.53),
    ("250-1", UNSATISFIABLE, 23.88),
    ("250-2", UNSATISFIABLE, 70.3),
    ("250-3", SATISFIABLE, 21.42),
    ("250-4", SATISFIABLE, 2.78),
    ("250-5", UNSATISFIABLE, 25.4),
    ("250-6", UNSATISFIABLE, 117.89),
    ("250-7", SATISFIABLE, 2.18),
    ("250-8", UNSATISFIABLE, 28.53),
    ("250-9", SATISFIABLE, 14.93),
    ("250-10", SATISFIABLE, 5),
    ("300-1", SATISFIABLE, 63.54),
    ("300-2", UNSATISFIABLE, 1444.74),
    ("300-3", UNSATISFIABLE, 284.11),
    ("300-4", UNSATISFIABLE, 301.25),
    ("300-5", SATISFIABLE, 789.79),
    ("300-6", UNSATISFIABLE, 443.43),
    ("300-7", SATISFIABLE, 7.52),
    ("300-8", UNSATISFIABLE, 920.41),
    ("300-9", SATISFIABLE, 295.42),
    ("300-10", SATISFIABLE, 201.8),
)

class Test(object):
    def __init__(self, ident, filename, result, time):
        self.ident = ident
        self.filename = filename
        self.result = result
        self.time = time

    def __cmp__(self, other):
        if (self.ident < other.ident): return -1
        if (self.ident > other.ident): return 1
        return 0

class Tester(object):
    def __init__(self, program='main.cc', prefix='', tests=tests):
        self.exec_name = '_test.exe_'
        self.program = program
        self.tests = [Test(i,os.path.join(prefix,'vars-%s.cnf'%i) ,r,t) for i,r,t in tests]

    def _compile(self):
        self.info("Compiling %s..." % self.program)
        cmd = subprocess.call(['g++', '-O3', '-o', self.exec_name, self.program])
        if cmd != 0:
            self.error("error\n")
        else:
            self.correct("done\n")

    def _loadtests(self, which):
        if not which:
            return self.tests
        tests = set()
        for t in which:
            tests.update({test for test in self.tests if t in test.ident})
        return sorted(tests)

    def prepare(self, which):
        self._compile()
        return self._loadtests(which)

    def clean(self):
        self.info("Cleaning up...")
        cmd = subprocess.call(['rm', self.exec_name])
        if cmd != 0:
            self.error("error\n")
        else:
            self.correct("done\n")

    def run(self, which):
        tests = self.prepare(which)
        for test in tests:
            self.info("Testing with %s...\n" % test.ident)
            with open(test.filename) as f:
                t = time.time()
                cmd = subprocess.call(['./%s'%self.exec_name], stdin=f)
                t = time.time() - t
            self._compare(test, cmd, t)
        self.clean()

    def _compare(self, test, res, time):
        if res == test.result:
            self.correct("CORRECT ")
            self.warning("%.3fs " % test.time)
            if test.time >= time:
                self.correct("%.3fs\n" % time)
            else:
                self.error("%.3fs\n" % time, fatal=False)
        else:
            self.error("WRONG\n")

    def error(self, msg, fatal=True):
        colorize(msg, 'red')
        if fatal:
            exit(1)

    def info(self, msg):
        colorize(msg, 'blue')

    def correct(self, msg):
        colorize(msg, 'green')

    def warning(self, msg):
        colorize(msg, 'yellow')

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(prog='sat-solver-tester')
    parser.add_argument('which', nargs='*', help='A string of the form 300 200-10...', default='')
    parser.add_argument('-d', '--dir', help='Directory where to find the tests.', default=os.getcwd())
    parser.add_argument('-f', '--file', help='File to compile and execute.', default='main.cc')

    args = vars(parser.parse_args(sys.argv[1:]))

    t = Tester(args.get('file'), args.get('dir'))
    t.run(args.get('which'))
