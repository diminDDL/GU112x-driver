extern crate repng;
extern crate scrap;

use scrap::{Capturer, Display};
use std::io::ErrorKind::WouldBlock;
//use std::fs::File;
use std::thread;
use std::time::Duration;

use serial2::SerialPort;


fn pritntBuffer(buffer: &[bool]){
    for i in 0..buffer.len() {
        if i % 16 == 0 {
            println!("");
        }
        if buffer[i] {
            print!("1");
        } else {
            print!("0");
        }
    }
    println!("");
}

fn main() {
    let target_x_size = 112;
    let target_y_size = 16;
    let threshold = 5;
    
    let port = SerialPort::open("/dev/ttyACM0", 115200);
    let mut buffer = [0; 256];
    match port {
        Ok(port) => {
            println!("Port opened");
            loop {
                //port.flush();
                let read = port.read(&mut buffer);
                match read {
                    Ok(read) => {
                        println!("{:?}", &buffer[..read]);
                    }
                    Err(e) => {
                        continue;
                    }
                }
                
                //let res = port.write(&buffer[..read]);
            }
        }
        Err(e) => {
            println!("Error opening port: {}", e);
        }
    }
        


    let one_second = Duration::new(1, 0);
    let one_frame = one_second / 60;

    let display = Display::primary().expect("Couldn't find primary display.");
    let mut capturer = Capturer::new(display).expect("Couldn't begin capture.");
    let (w, h) = (capturer.width(), capturer.height());

    //let x_step = (w as f32 / target_x_size as f32).floor() as usize;
    let x_step = 4;
    let y_step = x_step;
    println!("x_step: {}, y_step: {}", x_step, y_step);
    println!("Window size: {}, {}", x_step*target_x_size, y_step*target_y_size);
    thread::sleep(Duration::from_millis(2000));
    // the target display is 512x16 and has 1 bit per pixel
    // the buffer is 1920x1080 and has 4 bytes per pixel
    // the buffer is in BGRA format
    // we need to convert the buffer to 512x16 and 1 bit per pixel
    let mut target_buffer:Vec<bool> = vec![false; target_x_size * target_y_size];    

    let mut ser_buffer = [0; 256];

    let mut last_loop = std::time::Instant::now();
    loop {
        // Wait until there's a frame.

        let buffer = match capturer.frame() {
            Ok(buffer) => buffer,
            Err(error) => {
                if error.kind() == WouldBlock {
                    // Keep spinning.
                    thread::sleep(one_frame);
                    continue;
                } else {
                    panic!("Error: {}", error);
                }
            }
        };

        println!("Captured! tdelta: {}us", std::time::Instant::now().duration_since(last_loop).as_micros());
        last_loop = std::time::Instant::now();

        let stride = buffer.len() / h;

        let mut iter = 0;
        for x in 0..target_x_size {
            for y in 0..target_y_size {
                let bigX = x * x_step;
                let bigY = y * y_step;
                if bigX >= w || bigY >= h {
                    continue;
                }
                let i = stride * bigY + 4 * bigX;
                let r = buffer[i + 2];
                let g = buffer[i + 1];
                let b = buffer[i];
                let pixel = ((r as u16 + g as u16 + b as u16)/4) > threshold;
                target_buffer[iter] = pixel;
                iter += 1;
                //print!("{}", if pixel { "1" } else { "0" })
                //println!("{} {} {} | {} - {} {} | {} {}", r, g, b, pixel, bigX, bigY, x, y);
            }
            //println!()
        }
        
        //pritntBuffer(&target_buffer);

        // now we send the data
        // iterate in 16 bit chunks
        for i in 0..target_buffer.len() / 16 {
            let mut data = 0;
            for j in 0..16 {
                if target_buffer[i*16 + j] {
                    data |= 1 << j;
                }
            }
            //println!("{:#018b}", data);
            // split into 2 bytes
            let LS = (data & 0xFF) as u8;
            let MS = ((data >> 8) & 0xFF) as u8;

        }

        //break;
    }
}