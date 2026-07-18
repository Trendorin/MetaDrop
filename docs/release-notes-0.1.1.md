## MetaDrop 0.1.1

This privacy and packaging update replaces the original PDF trailer identifier during cleaning and verifies that the rewritten file received a different value.

### Security and correctness

- Regenerate the PDF `/ID` value instead of preserving a source identifier that could link revisions or copies.
- Add a qpdf-backed integration test covering document information removal and identifier replacement.
- Continue to require a successful reopen and post-clean metadata scan before saving any result.

### Packaging

- Install the MetaDrop license, privacy policy, security policy, and third-party notices with binary packages.
- Include exact bundled-library copyright files in the AppImage through linuxdeploy.
- Declare the Qt SVG runtime dependency for Debian/Ubuntu installations.
- Produce deterministic artifact lists and SHA-256 checksum manifests.

### Download

- `MetaDrop-0.1.1-x86_64.AppImage` — portable Linux build.
- `metadrop_0.1.1_amd64.deb` — Debian/Ubuntu package.
- `MetaDrop-0.1.1-source.tar.gz` — release source.
- `SHA256SUMS` — integrity checks for every artifact.

Metadata cleaning reduces accidental disclosure but does not anonymize visible content, filenames, filesystem records, or the account used to share a file. Review the [verified format scope](https://github.com/Trendorin/MetaDrop#verified-format-scope) before processing sensitive material.
