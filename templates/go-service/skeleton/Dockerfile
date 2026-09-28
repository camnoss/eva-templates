FROM golang:1.27 AS build

WORKDIR /src

COPY go.mod go.sum ./
RUN go mod download

COPY . .
RUN CGO_ENABLED=0 go build -trimpath -ldflags="-s -w" -o /out/api ./cmd/api

FROM gcr.io/distroless/static-debian13:nonroot

COPY --from=build /out/api /api

# Numeric UID of distroless "nonroot", so Kubernetes can enforce runAsNonRoot.
USER 65532:65532
EXPOSE 8080

ENTRYPOINT ["/api"]
