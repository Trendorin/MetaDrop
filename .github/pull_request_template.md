## Summary

Describe the behavior changed and why it belongs in MetaDrop.

## Validation

- [ ] `cmake --build --preset dev`
- [ ] `ctest --preset dev`
- [ ] `ctest --preset asan`
- [ ] Synthetic fixtures only; no personal or confidential files
- [ ] README format boundaries and translations updated when needed

## Safety review

- Does this alter source-file handling, output paths, verification, permissions, parsers, or supported-format claims?
- What failure case proves the application will refuse a false clean result?
