# Security model

MetaDrop treats every input file and embedded metadata structure as untrusted.

## Protected assets

- source files and unrelated user documents;
- confidentiality of inspected filenames, paths and metadata;
- integrity of the sanitized output and its verification result;
- predictable resource use while parsing archives and native media formats.

## Trust boundaries

| Boundary | Untrusted input | Control |
|---|---|---|
| File selection | paths, links and special files | Only regular files are accepted; symbolic links are refused. |
| Metadata parser | malformed images, audio, PDF and archives | Dedicated backends, strict failure states and process-level memory hardening. |
| Staging output | destination names and concurrent filesystem changes | Random owner-only staging file; source and output paths cannot match. |
| Office archives | entry names, counts and expanded sizes | Traversal, entry-count, part-size, total-size and file-type limits. |
| Verification | backend output and residual fields | The generated file is reopened and rescanned before final placement. |

## Sanitization flow

The source path is revalidated immediately before cleaning. A backend writes to a randomly named owner-only staging file in the destination directory. The source is never overwritten in version 0.1.x.

The same format backend must reopen the staging file and report zero fields that it classifies as removable. PDF output must also receive a new trailer identifier when the source contained `/ID`. Only after successful verification is the staging file atomically renamed into its final path.

Metadata work runs outside the UI thread and each operation uses independent backend objects. CI compiles with strict warnings and runs AddressSanitizer and UndefinedBehaviorSanitizer; CodeQL analyzes main-branch changes.

## Verification boundary

“Verified” means the active backend reopened the generated file and found none of the fields that backend marks removable. It is not proof that arbitrary bytes contain no identifying information.

Visible content, filenames and filesystem records, PDF annotations/forms/attachments, office comments/revisions/hidden content, macros and MP4 timed metadata tracks remain outside the current cleaning scope. The exact per-format boundary is listed in the README.

## Remaining risks

- Native media parsers can contain vulnerabilities; keep Qt, Exiv2, TagLib, qpdf, libarchive and MetaDrop updated.
- A process running as the same user can modify files or destination directories concurrently.
- Backups, snapshots, cloud history and sharing accounts can retain identifiers independently of MetaDrop.
- High-risk hostile samples should be handled in a disposable sandboxed user session or virtual machine.

Report security problems through [SECURITY.md](../SECURITY.md).
