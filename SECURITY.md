# Security policy

## Supported versions

| Version | Security fixes |
|---|---|
| 0.1.x | Yes |
| Development snapshots | Best effort |

## Report a vulnerability

Do not open a public issue for a vulnerability that could expose files, execute code, bypass verification, follow an unintended path, or produce a false “clean” result.

Use [GitHub private vulnerability reporting](https://github.com/Trendorin/MetaDrop/security/advisories/new). Include:

- affected version and Linux distribution;
- file family and backend involved;
- minimal reproduction steps;
- expected and observed result;
- sanitizer trace or debugger output when available;
- a synthetic proof-of-concept file with no personal or confidential data.

The project aims to acknowledge a complete report within 72 hours. A fix, coordinated disclosure date, and credit are agreed with the reporter based on severity and complexity.

## Security model

MetaDrop treats every input file as untrusted.

- Only regular files are accepted; symbolic links are refused.
- The source path is revalidated immediately before cleaning.
- Cleaning happens in a randomly named owner-only staging file.
- Output paths must not exist and can never equal the source path.
- Office containers have entry-count, metadata-part-size, expanded-size, path-traversal, and file-type checks.
- A backend must reopen the staging file and report zero removable fields before the result is installed.
- PDF output must contain a newly generated trailer identifier when the source had an `/ID` value.
- The final rename occurs within the destination directory to keep it atomic on the same filesystem.
- Metadata work runs off the UI thread; each operation uses independent backend objects.
- CI builds with strict warnings and runs AddressSanitizer plus UndefinedBehaviorSanitizer.

MetaDrop depends on mature parsers, but native media parsers can still contain vulnerabilities. Keep Exiv2, TagLib, qpdf, libarchive, Qt, and MetaDrop updated. For hostile samples, use a sandboxed disposable user session or virtual machine.

## Verification boundary

“Verified” means that the same format backend reopened the generated file and found none of the fields that backend marks removable. It is not a mathematical proof that arbitrary bytes contain no identifying information.

The exact per-format boundary is documented in the README. Visible content, filename and filesystem records, PDF annotations/forms/attachments, office comments/revisions/hidden content, and MP4 timed metadata tracks are outside version 0.1.x cleaning scope.
