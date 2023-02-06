extern crate repng;
extern crate scrap;

use scrap::{Capturer, Display};
use std::io::ErrorKind::WouldBlock;
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

    let x_offset = 200;
    let y_offset = 250;

    let threshold = 50;
    let smoothing = false;

    let magic:char = 'A';

    //let x_step = (w as f32 / target_x_size as f32).floor() as usize;
    let x_step = 1;
    let y_step = x_step;

    let port = SerialPort::open("/dev/ttyACM0", 115200);
    let mut ser_port;
    match port {
        Ok(port) => {
            println!("Port opened");
            ser_port = port;
        }
        Err(e) => {
            println!("Error opening port: {}", e);
            // exit the program
            return;
        }
    }

    let one_second = Duration::new(1, 0);
    let one_frame = one_second / 60;

    let display = Display::primary().expect("Couldn't find primary display.");
    let mut capturer = Capturer::new(display).expect("Couldn't begin capture.");
    let (w, h) = (capturer.width(), capturer.height());

    println!("x_step: {}, y_step: {}", x_step, y_step);
    println!("Window size: {}, {}", x_step*target_x_size, y_step*target_y_size);
    //thread::sleep(Duration::from_millis(2000));

    let mut target_buffer:Vec<bool> = vec![false; target_x_size * target_y_size];    

    let mut last_loop = std::time::Instant::now();
    loop {

        // Wait until there's a frame.
        let buffer = match capturer.frame() {
            Ok(buffer) => buffer,
            Err(error) => {
                if error.kind() == WouldBlock {
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
                let bigX = x * x_step + x_offset;
                let bigY = y * y_step + y_offset;
                if bigX >= w || bigY >= h {
                    continue;
                }
                let mut pixel = false;
                if smoothing{
                    let mut accumulator = 0;
                    for i in 0..x_step {
                        for j in 0..y_step {
                            let i = stride * (bigY + j) + 4 * (bigX + i);
                            let r = buffer[i + 2];
                            let g = buffer[i + 1];
                            let b = buffer[i];
                            accumulator += (r as u16 + g as u16 + b as u16)/3;
                        }
                    }
                    pixel = accumulator/((x_step*y_step) as u16) > threshold;
                }else{
                    let i = stride * bigY + 4 * bigX;
                    let r = buffer[i + 2];
                    let g = buffer[i + 1];
                    let b = buffer[i];
                    pixel = ((r as u16 + g as u16 + b as u16)/3) > threshold;
                }
                target_buffer[iter] = pixel;
                iter += 1;
            }
        }
        

        // now we create the u8 buffer to be sent
        let mut data_buffer:Vec<u8> = vec![0; target_buffer.len() / 8];
        // iterate in 16 bit chunks
        for i in 0..target_buffer.len() / 16 {
            let mut data = 0;
            for j in 0..16 {
                if target_buffer[i*16 + j] {
                    data |= 1 << j;
                }
            }
            // split into 2 bytes
            let LS = (data & 0xFF) as u8;
            let MS = ((data >> 8) & 0xFF) as u8;
            data_buffer[i*2] = LS;
            data_buffer[i*2 + 1] = MS;
        }

        loop {
            //ser_port.flush();
            let mut buffer = [0; 1];
            let read = ser_port.read(&mut buffer);
            match read {
                Ok(read) => {
                    // if we read a magic char, we can send the data
                    if buffer[0] as char == magic {
                        ser_port.write(&data_buffer);
                        break;
                    }
                }
                Err(e) => {
                    continue;
                }
            }
        }

        //break;
    }
}
