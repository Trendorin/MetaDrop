# Privacy and data handling

MetaDrop is designed for local file processing.

## Data flow

- Files and metadata remain on the local machine.
- The application has no HTTP client, telemetry SDK, analytics endpoint, account system, advertising code, or update tracker.
- MetaDrop does not upload filenames, paths, hashes, crash reports, or usage events.
- Desktop notifications contain only the source filename and local completion state.

## Local storage

MetaDrop stores only user preferences through `QSettings`: output mode, selected output directory, language, tray behavior, notification preference, and cleaning options. It does not keep a history of inspected files.

Cleaning uses a randomly named staging file in the destination directory. The staging file is created with owner-only permissions, removed on a handled failure, and renamed into the final path only after a successful rescan. A process crash or power loss can leave a `.metadrop-*` staging file in that directory; it can be deleted after confirming MetaDrop is not running.

The source file is never overwritten in version 0.1.x. Cleaned copies default to owner-only permissions. Preserving source file-system timestamps is optional and disabled by default.

PDF rewriting removes the source trailer file identifier. qpdf assigns a fresh identifier to the cleaned copy so it cannot be correlated through the original `/ID` value.

## What metadata cleaning cannot hide

MetaDrop does not promise anonymity. Personal information can remain in visible pixels, audio, document text, comments, tracked revisions, hidden content, attachments, filenames, surrounding directory names, file-system records, backups, cloud-provider history, or the account used to share a file.

For high-risk publication, review the cleaned content manually, rename the output, and use an appropriate isolated environment and sharing account.
