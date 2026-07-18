# Security policy

## Supported versions

| Version | Security updates |
|---|:---:|
| 0.1.x | Yes |
| Older / unreleased snapshots | No |

## Report a vulnerability

Use [GitHub private vulnerability reporting](https://github.com/Trendorin/MetaDrop/security/advisories/new). Do not open a public issue for a suspected vulnerability before a fix is available.

Include:

- affected MetaDrop version and Linux distribution;
- file family and metadata backend involved;
- minimal reproduction steps and expected/observed behavior;
- whether verification was bypassed or an unintended path was read or changed;
- logs with usernames, paths, tokens and unrelated personal data removed;
- a synthetic proof-of-concept file containing no confidential data.

The target response time is 72 hours for acknowledgement and 14 days for an initial assessment. Complex fixes or coordinated disclosure can take longer. Credit is given with the reporter's consent.

High-priority reports include parser-driven code execution, path traversal, symlink following, source overwrite, output installed without verification, unsafe archive expansion and release-workflow compromise.

The complete trust boundary is documented in the [security model](docs/SECURITY_MODEL.md).
