

# Contributing to tdnf

The tdnf project team welcomes contributions from the community. If you wish to contribute code and you have not signed our contributor license agreement (CLA), our bot will update the issue when you open a Pull Request. For any questions about the CLA process, please refer to our [FAQ](https://cla.vmware.com/faq).

## Contribution Flow

This is a rough outline of what a contributor's workflow looks like:

- Create a topic branch from where you want to base your work
- Make commits of logical units
- Make sure your commit messages are in the proper format (see below)
- Push your changes to a topic branch in your fork of the repository
- Submit a pull request

Example:

``` shell
git remote add upstream https://github.com/vmware/tdnf.git
git checkout -b my-new-feature master
git commit -a
git push origin my-new-feature
```

### Testing Changes

tdnf comes with an extensive test suite. Before creating a PR or making any changes, please test with these commands:
```
export DIST=photon
cd ci
docker build -t photon/tdnf-build -f Dockerfile.photon .
cd ..
docker run --security-opt seccomp:unconfined --rm -it -e DIST -v$(pwd):/build -w/build ${DIST}/tdnf-build ./ci/docker-entrypoint.sh
```

### Staying In Sync With Upstream

When your branch gets out of sync with the vmware/main branch, use the following to update:

``` shell
git checkout my-new-feature
git fetch -a
git pull --rebase upstream main
git push --force-with-lease origin my-new-feature
```

### Updating pull requests

If your PR fails to pass CI or needs changes based on code review, you'll most likely want to squash these changes into
existing commits.

If your pull request contains a single commit or your changes are related to the most recent commit, you can simply
amend the commit.

``` bash
git add .
git commit --amend
git push --force-with-lease origin my-new-feature
```

If you need to squash changes into an earlier commit, you can use:

``` bash
git add .
git commit --fixup <commit>
git rebase -i --autosquash main
git push --force-with-lease origin my-new-feature
```

Be sure to add a comment to the PR indicating your new changes are ready to review, as GitHub does not generate a
notification when you git push.

### Code Style

Please follow the existing coding style in the files you are changing.


### Formatting Commit Messages

We follow the conventions on [How to Write a Git Commit Message](http://chris.beams.io/posts/git-commit/).

Be sure to include any related GitHub issue references in the commit message.  See
[GFM syntax](https://guides.github.com/features/mastering-markdown/#GitHub-flavored-markdown) for referencing issues
and commits.

## Reporting Bugs and Creating Issues

When opening a new issue, try to roughly follow the commit message format conventions above.
