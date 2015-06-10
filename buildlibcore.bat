
rustc -C opt-level=2 -Z no-landing-pads --cfg i686-unknown-linux-gnu --target=target.json -g libcore/lib.rs --out-dir bin/