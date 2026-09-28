# Eva Templates

> Every Eva Unit starts here. Golden paths to launch new services into Geofront, from a single form in MAGI.

**Eva Templates** holds the [Backstage Software Templates](https://backstage.io/docs/features/software-templates/) of **Nerv**, an internal developer platform (IDP) built on Kubernetes, Backstage and ArgoCD.

Each template is a **golden path**: an opinionated, supported way to create a new service. With a single form in MAGI, a team gets:

- A new repository with working code, a Dockerfile and CI already in place
- A pull request in Central Dogma that deploys the service to `dev` and merges itself once the first image is built
- Automatic deploys to `dev` after every merge to `main`, through the Central Dogma release bot
- The service registered in the catalog, with its owner, Kubernetes status and ArgoCD deployments

No tickets, no copy-pasting from another repo, no waiting on the platform team.


## Where Eva Templates fits in Nerv

| Codename | Component | Repository |
|---|---|---|
| **Geofront** | Kubernetes cluster(s) | — |
| **Central Dogma** | GitOps source of truth (ArgoCD) | [`central-dogma`](https://github.com/camnoss/central-dogma) |
| **MAGI** | Developer portal (Backstage) | [`magi`](https://github.com/camnoss/magi) |
| **Eva Templates** | Golden path templates | this repository |
| **Eva Units** | Team workloads | created from these templates |

## Template conventions

- **Names** are lowercase and hyphenated, describing the stack: `go-service`, `node-service`, `static-site`.
- **Every template must produce a deployable service.** A template that stops at "repository created" is not a golden path.
- **Every generated repository must include** `catalog-info.yaml` with an `owner`, and the `backstage.io/kubernetes-id` and `argocd/app-name` annotations.
- **Every template must connect to the release bot:** run `nerv:release-bot:connect` after `publish:github`, and end CI with a `release` job that sends an `image-published` dispatch to Central Dogma (see `go-service`). CI must tag images with the full commit SHA, which is what the bot deploys.
- **Secure by default:** non-root containers, resource requests and limits, and health probes in the manifests.
- **Keep templates minimal.** Ship the smallest service that builds, deploys and passes health checks. Teams add features; templates provide the path.
- **Templates are self-contained.** Each one owns its `skeleton/` and `deploy/`, even where files look alike, so changing one never changes another.


## BRM templates

`brm-custom-fm` (a new Facility Module with custom opcodes) and `brm-policy-override` (a customized stock policy opcode) scaffold Oracle BRM 15 code in C.

- **An opcode ships as a Connection Manager.** Opcodes run inside a CM, so each repository builds a CM image (`FROM` the BRM 15 CM image, plus its library) and deploys it as its own service. That keeps the golden path contract: a deployable service, released by the Central Dogma release bot.
- **Build contract:** both templates build inside the BRM SDK image with `make` (library in `build/lib/`), `make test` (cmocka unit tests) and `make lint` (clang-format and cppcheck), through the Dockerfile's `test` and `runtime` stages.
- **Code layout:** `src/ops/` holds thin opcode handlers, the only code that makes PCM calls. `src/logic/` holds the flist logic that unit tests cover without a CM.
- **Oracle source is never committed.** `brm-policy-override` copies the stock `fm_*_pol` source from the SDK at build time and routes the opcode to a wrapper that calls the stock implementation.
- **Platform prerequisites:** BRM core (DM and database) running in Geofront, the `brm-cm-credentials` Secret in each service namespace, and the SDK and CM base images (`BRM_SDK_IMAGE` and `BRM_CM_IMAGE` in the Dockerfiles).
- **Licensing:** the BRM SDK and CM images contain Oracle proprietary software. Publishing them, or images built from them, on a public registry is redistribution that Oracle's license generally does not allow. Use private images outside an educational setup.


## Templating gotchas

Backstage renders files with [Nunjucks](https://mozilla.github.io/nunjucks/) using `${{ }}`, the same syntax as GitHub Actions expressions. Files that contain Actions expressions must be excluded from templating:

```yaml
- id: fetch-skeleton
  action: fetch:template
  input:
    url: ./skeleton
    copyWithoutTemplating:
      - .github/workflows/*
```

Alternatively, escape individual expressions: `${{ '${{ github.sha }}' }}`.

## Useful links

- [Backstage Software Templates](https://backstage.io/docs/features/software-templates/)
- [Built-in scaffolder actions](https://backstage.io/docs/features/software-templates/builtin-actions)
- [Central Dogma](https://github.com/camnoss/central-dogma)
- [MAGI](https://github.com/camnoss/magi)