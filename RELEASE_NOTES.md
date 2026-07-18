# MetaDrop 0.1.4

MetaDrop 0.1.4 corrects Fedora packaging. Application behavior and the verified metadata-cleaning boundary are unchanged.

## Included

| Asset | Purpose |
|---|---|
| `metadrop_0.1.4_amd64.deb` | Debian / Ubuntu package |
| `metadrop-0.1.4-1.x86_64.rpm` | Fedora 44 package |
| `PKGBUILD` | Arch-family package recipe |
| `MetaDrop-0.1.4-x86_64.tar.gz` | Installable Linux tree |
| `MetaDrop-0.1.4-source.tar.gz` | Release source |
| `MetaDrop-0.1.4.spdx` | SPDX 2.3 software bill of materials |
| `SHA256SUMS` | Integrity manifest for every asset |

## Fixed

- The RPM is now compiled inside Fedora 44 and links against Fedora-provided `libexiv2.so.28` and `libqpdf.so.30`.
- The release workflow installs the finished RPM through DNF and checks every dynamic-library link before publication.
- Debian/Ubuntu and Fedora packages are built in separate native environments.

The incompatible 0.1.3 RPM should not be used on Fedora. Download the 0.1.4 RPM and install it with:

```bash
sudo dnf install ./metadrop-0.1.4-1.x86_64.rpm
```

Verify downloads with `sha256sum --ignore-missing --check SHA256SUMS`.
