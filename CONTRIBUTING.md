

# Contributing to tdnf

The tdnf project team welcomes contributions from the community. If you wish to contribute code and you have not signed our contributor license agreement (CLA), our bot will update the issue when you open a Pull Request. For any questions about the CLA process, please refer to our [FAQ](https://cla.vmware.com/faq).

## Contribution Flow

This is a rough outline of what a contributor's workflow looks like:

- Create a fork on github
- Create a topic branch from where you want to base your work
- Make commits of logical units
- Make sure your commit messages are in the proper format (see below)
- Push your changes to a topic branch in your fork of the repository
- Submit a pull request

Example:

``` shell
git remote add yourfork https://github.com/yourname/tdnf.git
git pull yourfork
git checkout -b topic/yourname/feature
... hack ...
... test, see below ...
git add path/to/files
git commit -a
git push yourfork my-new-feature
```
On the push, you should see a URL to create a pull request (PR) on github. You can also go to the github page of your fork, select your topic branch and create the PR from there.

### Testing Changes

tdnf comes with an extensive test suite. Before creating a PR or making any changes, please test with these commands:
```
export DIST=photon
cd ci
docker build -t photon/tdnf-build -f Dockerfile.photon .
cd ..
docker run --security-opt seccomp:unconfined --rm -it -e DIST -v$(pwd):/build -w/build ${DIST}/tdnf-build ./ci/docker-entrypoint.sh
```

The repo contains workflows in the `.github` directory. These will be executed whenever the PR changes. If you never contributed before, this needs to be approved by the maintainers (if that doesn't happen within a reasonable time, feel free to ping them). Please observe the results of these tests, and address errors in your PR.

### Staying In Sync With Upstream

When your branch gets out of sync with the `dev` branch, use the following to update:

``` shell
git checkout topic/yourname/feature
git fetch -a
git rebase dev
git push -f yourrepo
```

Your changes will automatically update in the PR.

### Updating pull requests

If your PR fails to pass CI or needs changes based on code review, you'll most likely want to squash these changes into
existing commits.

If your pull request contains a single commit or your changes are related to the most recent commit you can simply
amend the commit.

``` bash
git add .
git commit --amend
git push -f yourfork
```

If you need to squash changes into an earlier commit, you can use:

``` bash
git add path/to/files
git commit
git rebase -i dev
git push -f yourfork
```

If the changes make sense on their own, you can also just add the commit, in which case you don't need the `-f` option on `push`.

The rule of thumb is that a single commit can be merged on its own. For example, if you need to add a helper function that can be used by other changes, this can be in one commit. Also, if you discovered a bug in existing code this can be fixed in one commit. If however it is bug fix of one your changes, squash it into the commit that added it.

The reason for this is that it should be easy to follow changes made when looking at the history later (this may be years in the future). It should also be easy to revert changes in case they cause issues, without having too much impact on other code. Bug fixes may also be crossported to other maintenance branches.

Be sure to add a comment to the PR indicating your new changes are ready to review, as GitHub does not generate a
notification when you git push.

### Code Style

Please follow the existing coding style in the files you are changing - even if it feels weird. A consistent style makes it easier to follow the code.

### Formatting Commit Messages and PR Description

Commit messages should adequately describe the change. This can be one line or more.

Use the PR description for more details. For example, describe new commands or flags added, with examples. If applicable, it should be possible to copy straight to the wiki.

## Reporting Bugs and Creating Issues

When opening a new issue, try to roughly follow the commit message format conventions above.
