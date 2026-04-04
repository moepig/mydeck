import os.path
Import("env")
# build rv003usb.S in a separate folder to avoid name clash with rv003usb.c
# see: https://github.com/cnlohr/rv003usb/blob/master/.github/build_rv003usb.py
env.BuildSources(
    os.path.join("$BUILD_DIR", "rv003usbASM"),
    os.path.join(env.subst("$PROJECT_DIR"), "ext_src", "rv003usb", "rv003usb"),
    src_filter=[
        "-<*>",
        "+<rv003usb.S>",
    ]
)
