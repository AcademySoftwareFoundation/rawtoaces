# Contributing to rawtoaces

Thank you for your interest in contributing to rawtoaces. This document
explains our contribution process and procedures:

* [Getting Information](#Getting-Information)
* [Legal Requirements](#Legal-Requirements)
* [Development Workflow](#Development-Workflow)
* [Coding Style](#Coding-Style)
* [Versioning Policy](#Versioning-Policy)

For a description of the roles and responsibilities of the various
members of the rawtoaces community, see the rawtoaces project's [Technical
Charter](https://github.com/AcademySoftwareFoundation/foundation/blob/main/project_charters/rawtoaces_charter.pdf). Briefly,
a "contributor" is anyone who submits content to the project, a
"committer" reviews and approves such submissions, and the "Technical
Steering Committee" provides general project oversight and governance.

## Getting Information

The primary ways to connect with the rawtoaces project are:

* [GitHub Issues](https://github.com/AcademySoftwareFoundation/rawtoaces/issues): GitHub Issues are used both to track bugs and to discuss feature requests.
  
* The [ASWF Slack](https://slack.aswf.io/) has a `rawtoaces` channel. Sign up
for the Slack on your own, then under "channels", select "browse channels" and
you should see the rawtoaces channel (among those of the other projects and
working groups).

* The [rawtoaces-discussion](https://lists.aswf.io/g/rawtoaces-discussion) mail list:
  You can sign up for the mail list on your own using the link above.

### How to Ask for Help

If you have trouble installing, building, or using rawtoaces, but
there's not yet reason to suspect you've encountered a genuine bug,
start by posting a question to the slack channel or the mail list.

### How to Report a Bug

rawtoaces use GitHub's issue tracking system for bugs and enhancements:
https://github.com/AcademySoftwareFoundation/rawtoaces/issues

If you are submitting a bug report, please be sure to note which
version of rawtoaces you are using, on what platform (OS/version, which
compiler you used, and any special build flags or other unusual
environmental issues). Please give a specific account of

* what you tried
* what happened
* what you expected to happen instead

with enough detail that others can reproduce the problem.

### How to Request a Change

Open a GitHub issue: https://github.com/AcademySoftwareFoundation/rawtoaces/issues.

Describe the situation and the objective in as much detail as
possible. Feature requests will almost certainly spawn a discussion
among the project community.

### How to Report a Security Vulnerability

If you think you've found a potential vulnerability in rawtoaces, please
refer to [SECURITY.md](SECURITY.md) to responsibly disclose it.

### How to Contribute a Bug Fix or Change

To contribute code to the project, you will need:

* A good knowledge of git.

* A fork of the GitHub repo.

* An understanding of the project's development workflow.

* Legal authorization, that is, you need to have signed a contributor
  License Agreement. See below for details.

## Legal Requirements

rawtoaces is a project of the Academy Software Foundation and follows the
open source software best practice policies of the Linux Foundation.

### License

rawtoaces is licensed under the [Apache-2.0](LICENSE.md)
license. Contributions to the library should abide by that standard
license.

### Contributor License Agreements

To protect the project -- and the contributors! -- we do require a Contributor
License Agreement (CLA) for anybody submitting changes. This is for your own
safety, as it prevents any possible future disputes between code authors and
their employers or anyone else who might think they might own the IP output of
the author.

The easiest way to sign CLAs is digitally [using
EasyCLA](https://corporate.v1.easycla.lfx.linuxfoundation.org). There are
detailed step-by-step instructions about using the EasyCLA system for
[corporate CLAs](https://docs.linuxfoundation.org/lfx/easycla/v2-current/contributors/corporate-contributor)
and [individual CLAs](https://docs.linuxfoundation.org/lfx/easycla/v2-current/contributors/individual-contributor#github).

* If you are an individual writing the code on your own time and
  you're **sure** you are the sole owner of any intellectual property you
  contribute, you can sign the CLA as an **Individual Contributor**.

* If you are writing the code as part of your job, or if your employer
  retains ownership to intellectual property you create, no matter how
  small, then your company's legal affairs representatives should sign
  a **Corporate Contributor Licence Agreement**. If your company already
  has a signed CCLA on file, ask your local CLA manager to add you
  (via your GitHub account name/email address) to your company's
  "approved" list.

### Commit Sign-Off

Every commit must be signed off.  That is, every commit log message
must include a “`Signed-off-by`” line (generated, for example, with
“`git commit --signoff`”), indicating that the committer wrote the
code and has the right to release it under the [Apache-2.0](LICENSE.md)
license. See https://github.com/AcademySoftwareFoundation/tac/blob/main/process/contributing.md#contribution-sign-off for more information on this requirement.

## Development Workflow

### Git Basics

Working with rawtoaces requires understanding a significant amount of
Git and GitHub based terminology. If you’re unfamiliar with these
tools or their lingo, please look at the [GitHub
Glossary](https://help.github.com/articles/github-glossary/) or browse
[GitHub Help](https://help.github.com/).

To contribute, you need a GitHub account. This is needed in order to
push changes to the upstream repository, via a pull request.

You will also need Git installed on your local development machine. If
you need setup assistance, please see the official [Git
Documentation](https://git-scm.com/doc).

### Repository Structure and Commit Policy

The rawtoaces repository uses a simple branching and merging strategy.

All development work is done directly on the ``main`` branch. The ``main``
branch represents the bleeding-edge of the project and most
contributions should be done on top of it.

After sufficient work is done on the ``main`` branch and the rawtoaces
leadership determines that a release is due, we will bump the relevant
internal versioning and tag a commit with the corresponding version
number, e.g. v2.0.1. Each minor version also has its own “Release
Branch”, e.g. dev-1.1. This marks a branch of code dedicated to that
``major.minor`` version, which allows upstream bug fixes to be
cherry-picked to a given version while still allowing the ``main``
branch to continue forward onto higher versions. This basic repository
structure keeps maintenance low, while remaining simple to understand.

To reiterate, the ``main`` branch represents the latest development
version, so beware that it may include untested features and is not
generally stable enough for release.  To retrieve a stable version of
the source code, use one of the release branches.

### The Git Workflow

This development workflow is sometimes referred to as
[OneFlow](https://www.endoflineblog.com/oneflow-a-git-branching-model-and-workflow). It
leads to a simple, clean, linear edit history in the repo.

The rawtoaces GitHub repo allows rebase merging and disallows merge
commits and squash merging. This ensures that the repo edit history
remains linear, avoiding the "bubbles" characteristic of the
[GitFlow](https://www.endoflineblog.com/gitflow-considered-harmful)
workflow.

### Forks

In a typical workflow, you should **fork** the rawtoaces repository to
your account. This creates a copy of the repository under your user
namespace and serves as the “home base” for your development branches,
from which you will submit **pull requests** to the upstream
repository to be merged.

Once your Git environment is operational, the next step is to locally
**clone** your forked rawtoaces repository, and add a **remote**
pointing to the upstream rawtoaces repository. These topics are
covered in the GitHub documentation [Cloning a
repository](https://help.github.com/articles/cloning-a-repository/)
and [Configuring a remote for a
fork](https://help.github.com/articles/configuring-a-remote-for-a-fork/).

### Pull Requests

Contributions should be submitted as Github pull requests. See
[Creating a pull request](https://help.github.com/articles/creating-a-pull-request/)
if you're unfamiliar with this concept. 

The development cycle for a code change should follow this protocol:

1. Create a topic branch in your local repository.

2. Make changes, compile, and test thoroughly. Code style should match existing
style and conventions, and changes should be focused on the topic the pull
request will be addressing. Make unrelated changes in a separate topic branch
with a separate pull request.

3. Push commits to your fork.

4. Create a Github pull request from your topic branch.

5. Pull requests will be reviewed by project committers and contributors,
who may discuss, offer constructive feedback, request changes, or approve
the work.

6. Upon receiving the required number of committer approvals (as
outlined in [Required Approvals](#required-approvals)), a committer
other than the PR contributor may merge changes into the ``main``
branch.

### Code Review and Required Approvals

Modifications of the contents of the rawtoaces repository are made on a
collaborative basis. Anyone with a GitHub account may propose a
modification via pull request and it will be considered by the project
committers.

Code review is a process where someone other than the author of a patch
examines the proposed changes and approves or critiques them. The main
benefits are to:

- Encourage submitters to ensure that their changes are well thought out,
  tested, and documented so that their intent and wisdom will be clear to
  others.
- Directly find shortcomings in or suggest improvements to the proposed
  changes, and ensure that the changes are consistent with the project's best
  practices.
- Minimize the amount of the code base that has only been seen or is only
  understood by one person.
- Improve security and robustness of the code base by assuring that at least
  two people (submitter and reviewer) agree that every patch is reasonable and
  safe.

### Test Policy

All functionality in the library must be covered by an automated
test.

* All new functionality should be accompanied by a test that validates
  its behavior.

* Any change to existing functionality should have tests added if they
  don't already exist.

The tests can be run locally via:

    cmake -S . -B build_test
    cmake --build build_test
    ctest --test-dir build_test

The tests also run on the CI on every push to a  pull request, and every change
to main.

## Coding Style

### File conventions

C++ implementation should be named `*.cpp`. Headers should be named `.h`.

All headers should contain:

    #pragma once

All new source files should begin with a copyright and license stating:

    // SPDX-License-Identifier: Apache-2.0
    // Copyright Contributors to the rawtoaces Project.

### Formatting

The coding style of the library source code is enforced via Clang format, with
the configuration defined in [.clang-format](../.github/.clang-format).

One of the CI test matrix entries runs clang-format and fails if any
diffs were generated (that is, if any of your code did not 100% conform to
the `.clang-format` formatting configuration). If it fails, clicking on that
test log will show you the diffs generated, so that you can easily correct
it on your end and update the PR with the formatting fixes.

Because the basic formatting is automated by clang-format, we won't
enumerate the rules here.

### Naming Conventions

* In general, classes and template type names should start with upper
  case and capitalize new words: `class CustomerList;`

* In general, local variables should use camelCase. Macros and
  constants should use `ALL_CAPS`.

If your class is extremely similar to, or modeled after, something in the
standard library, or something else we interoperate with, it's ok to
use their naming conventions. For example, very general utility classes and
templates (the kind of thing you would normally find in std) should
be lower case with underscores separating words, as they would be if they
were standards.

    template <class T> shared_ptr;
    class scoped_mutex;
    
### Third-party libraries

Prefer C++11 `std` over other libraries where possible. Check with the project
leadership before adding new dependencies.

### Comments and Doxygen

Comment philosophy: try to be clear, try to help teach the reader
what's going on in your code.

Prefer C++ comments (starting line with `//`) rather than C comments
(`/* ... */`).

For public APIs, use Doxygen-style comments (start with `///`), such as:

    /// Explanation of a class.  Note THREE slashes!
    /// Also, you need at least two lines like this.  If you don't have enough
    /// for two lines, make one line blank like this:
    ///
    class myclass {
        ....
        float foo;  ///< Doxygen comments on same line look like this
    }

## Versioning Policy

rawtoaces uses [semantic versioning](https://semver.org), which labels
each version with three numbers: ``major.minor.patch``, where:

* ``major`` - indicates incompatible API changes
* ``minor`` - indicates functionality added in a backwards-compatible manner
* ``patch`` - indicates backwards-compatible bug fixes 
