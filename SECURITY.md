# Security Policy

## Supported Versions

This gives guidance about which branches are supported with patches to
security vulnerabilities.

| Version / branch  | Supported                                            |
| ----------------- | ---------------------------------------------------- |
| main              | :white_check_mark: :construction: ALL fixes immediately, but this is a branch under development with a frequently unstable ABI and occasionally unstable API. |
| 1.1.x             | :white_check_mark: All fixes that can be backported without breaking ABI compatibility. |
| < 1.1.x           | :x: No longer receiving patches of any kind.        |


## Reporting a Vulnerability

If you think you've found a potential vulnerability in rawtoaces, please
report it by pressing the "Report a vulnerability" button on the 
[Security tab](https://github.com/AcademySoftwareFoundation/rawtoaces/security/advisories) 
of the rawtoaces project page on GitHub. Include detailed steps to reproduce the issue,
and any other information that could aid an investigation. Our policy is to
respond to vulnerability reports within 14 days.

Our policy is to address critical security vulnerabilities rapidly and post
patches as quickly as possible.


## Other security features

### Signed tags
Starting with rawtoaces 1.1.0, we cryptographically sign release tags.
To verify a tag, you can use the `git tag -v` command, which will check
the signature against the public key that is included in the repository.
For example,

```bash
git tag -v v1.1.0
```

## Outstanding Security Issues

None known


## History of CVE Fixes

None known
