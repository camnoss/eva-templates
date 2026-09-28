# ${{ values.name }}

${{ values.description }}

A Go service built with [Gin](https://gin-gonic.com/), created from the `go-service` golden path in MAGI.

## Run locally

Requires Go 1.27.

```bash
go run ./cmd/api
curl localhost:8080/healthz
```

The server listens on `PORT` (default `8080`).

| Endpoint | Purpose |
|---|---|
| `GET /` | Hello endpoint |
| `GET /healthz` | Liveness probe |
| `GET /readyz` | Readiness probe. Add dependency checks here. |

## Test

```bash
go vet ./...
go test ./...
```

## Build and deploy

- **Pull requests:** CI runs vet, tests and a Docker build.
- **Pushes to `main`:** CI pushes `ghcr.io/camnoss/${{ values.name }}` tagged with the commit SHA and `main`.
- **Deployment:** manifests live in [Central Dogma](https://github.com/camnoss/central-dogma) under `apps/${{ values.name }}/`. To release a new version, open a PR there that updates `newTag` in the overlay.

| Environment | Namespace | ArgoCD app |
|---|---|---|
| dev | `${{ values.name }}-dev` | `${{ values.name }}-dev` |
