FROM devkitpro/devkitppc:latest

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20260418 /artifacts $DEVKITPRO

WORKDIR /project
