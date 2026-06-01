# SEED-NOTES — KitchenDisplay

First architecture artifact (batch 2 of the producer backfill). See
`../../BATCH-2-FINDINGS.md` and `../../REVIEW-2.md` for the shared rationale; this
file records what is specific to this repo.

- **Producer id:** `kitchen-display`  ·  **introduced:** `2024-03-29` (repo's first commit, full clone).
- **What this repo owns:** the Kitchen Display kiosk product («SoftwareProduct» ApplicationComponent — native C++/LVGL on a Raspberry Pi).
- **Edges:** → `svc:trello-api` (external SaaS declared locally; hardcoded base URL → no boundBy).
- **Not an esp-mdm device.** It builds in Jenkins — the former `pipelines/kitchendisplay.groovy` was moved to the **root `Jenkinsfile`** (D6), with `Jenkinsfile.architecture` beside it. The physical Pi is currently powered off, but the product is live → **lifecycle: active** (a turned-off device, not a removed one).
- **Validation:** `./scripts/arch-validate.py docs/architecture/architecture.yaml` → OK.
