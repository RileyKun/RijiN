include!("VirtualizerSDK.rs");

fn main() {
    unsafe { VIRTUALIZER_START(); }
    println!("Hello from VIRTUALIZER macro!"); 
    unsafe { VIRTUALIZER_END(); }

    unsafe { VIRTUALIZER_TIGER_WHITE_START(); }
    println!("Hello from TIGER_WHITE macro!");
    unsafe { VIRTUALIZER_TIGER_WHITE_END(); }

    unsafe { VIRTUALIZER_FISH_WHITE_START(); }
    println!("Hello from FISH_WHITE macro!");
    unsafe { VIRTUALIZER_FISH_WHITE_END(); } 
}

