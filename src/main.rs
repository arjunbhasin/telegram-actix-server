use teloxide::prelude::*;
use actix_web::{get,web, App, HttpResponse, HttpServer, Responder};
use std::sync::{Arc, Mutex};
use std::env;

static ALLOWED_CHAT_ID:i64 = 1234567890; // replace with your chat id

#[get("/")]
async fn home_page() -> impl Responder {
    HttpResponse::Ok().body("Server is running!")
}

#[get("/get-string")]
async fn get_string(data: web::Data<Arc<Mutex<String>>>) -> impl Responder {
    // read the string to display
    let string_to_display = data.lock().unwrap();
    
    // return the string to display
    HttpResponse::Ok().body(string_to_display.to_string())
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {

    // default string to display
    let string_to_display = Arc::new(Mutex::new("Hello".to_string()));

    let string_to_display_for_thread = string_to_display.clone();

    // start an async thread to where the bot will listen for messages
    actix_rt::spawn(async move {
        let bot = Bot::from_env();

        // WARNING! Teloxide will crash if TELOXIDE_TOKEN is not set in environment variables
        teloxide::repl(bot, move |bot: Bot, msg: Message| {
            // clone the string_to_display to be used in the async block
            let string_to_display = string_to_display_for_thread.clone();

            async move {

                // Uncomment the following line to get the chat id of the user
                // println!("Got a message from: {}", msg.chat.id);

                if msg.chat.id == ChatId(ALLOWED_CHAT_ID.into()) {
                    // read the incoming message and take the first 10 characters
                    let text = &msg.text().unwrap_or_default()[..10];
                    {
                        // Lock the mutex and update string_to_display 
                        // within a block to drop the lock immediately after.
                        let mut locked_string = string_to_display.lock().unwrap();
                        *locked_string = text.to_string();
                    } // MutexGuard is dropped here
                    
                    // send a message to the user if the string was updated
                    bot.send_message(msg.chat.id, "String updated").await?;

                    return Ok(());
                }
                // else unauthorized
                bot.send_message(msg.chat.id, "Unauthorized!").await?;

                Ok(())
            }
        })
        .await;
    });

    // get the port from the environment variable (required for railway)
    let port = env::var("PORT").unwrap_or_else(|_| "8080".to_string());

    HttpServer::new(move || {
        App::new()
            .app_data(web::Data::new(string_to_display.clone()))
            .service(home_page)
            .service(get_string)

    })
    .bind(format!("0.0.0.0:{}", port))?
    .run()
    .await
}