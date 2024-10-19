use neon::prelude::*;

mod attach;
mod detach;
mod input;
mod reset;
mod window;

#[neon::main]
fn main(mut cx: ModuleContext) -> NeonResult<()> {
    cx.export_function("attach", attach::attach)?;
    cx.export_function("detach", detach::detach)?;
    cx.export_function("reset", reset::reset)?;
    Ok(())
}
