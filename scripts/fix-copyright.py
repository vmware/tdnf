#!/usr/bin/env python
#
# Copyright (C) 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
# Author: Oliver Kurth <okurth@vmware.com>
#
# This script will go through all files in a git repo and fix the copyright years
# in their headers.
#
# IMPORTANT:
# This script needs to be enhanced to exclude commit ids that were created
# by using this script. For example, if a file was corrected in the year 2022
# to have 2015-2017, the next run of this script would wrongly autocorrect the
# year to 2022.

import os
import re
from datetime import datetime

IGNORE_COMMITS = ['38fa1f466b86ffdf550b17461b2722b4ddc07a85']

class Commit:
    def __init__(self, id):
        self.id = id


def get_files():
    files = []
    stream = os.popen('git ls-files')
    for line in stream.readlines():
        files.append(line.strip())
    return files


def get_latest_commit(file):
    latest_commit = None
    # Commits are not necessarily in chronological order,
    # so we cannot just use the top most commit entry in the log
    stream = os.popen('git log --pretty=fuller {}'.format(file))
    for line in stream.readlines():
        if line.startswith('commit'):
            commit = Commit(line.strip().split(' ')[1])
        # We use the commit date, not the author date
        # see https://stackoverflow.com/questions/11856983/why-git-authordate-is-different-from-commitdate
        elif line.startswith('CommitDate:'):
            date_str = line[len('CommitDate:'):].strip()
            commit.date = datetime.strptime(date_str, '%a %b %d %H:%M:%S %Y %z')
            if latest_commit is None or commit.date > latest_commit.date:
                latest_commit = commit
    return latest_commit


def fix_file(file, actual_year):
    saved_mode = os.stat(file).st_mode
    tmpfile = "{}.cfix".format(file)
    with open(file, 'rt') as fin:
        with open(tmpfile, 'wt') as fout:
            for line in fin.readlines():

                # pattern with two years
                m = re.search('^(.*)Copyright \(C\) (\d{3,4})\w?[-,\s]\w?(\d{4}) VMware, Inc\. All Rights Reserved', line)
                if m is not None:
                    prefix = m.group(1)
                    y1 = m.group(2)
                    y2 = m.group(3)
                    fixed = "{prefix}Copyright (C) {y1}-{y2} VMware, Inc. All Rights Reserved.\n".format(prefix=prefix, y1=y1, y2=actual_year)
                    fout.write(fixed)
                    continue

                # pattern with just one year
                m = re.search('^(.*)Copyright \(C\) (\d{3,4}) VMware, Inc\. All Rights Reserved', line)
                if m is not None:
                    prefix = m.group(1)
                    y1 = m.group(2)
                    if y1 != actual_year:
                        fixed = "{prefix}Copyright (C) {y1}-{y2} VMware, Inc. All Rights Reserved.\n".format(prefix=prefix, y1=y1, y2=actual_year)
                    else:
                        fixed = "{prefix}Copyright (C) {y1} VMware, Inc. All Rights Reserved.\n".format(prefix=prefix, y1=y1)
                    fout.write(fixed)
                    continue

                fout.write(line)
    os.rename(tmpfile, file)
    os.chmod(file, saved_mode)

if __name__ == '__main__':
    files = get_files()
    for f in files:
        commit = get_latest_commit(f)
        if commit.id not in IGNORE_COMMITS:
            year = str(commit.date.year)
            fix_file(f, year)
