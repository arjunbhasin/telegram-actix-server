## Teleoxide + Actix Web server

Sample code for setting up a Teleoxide based Telegram bot along with an actix web server.

The application accepts a string sent in via Telegram bot (only first 10 chars) and updates an internal variable 'string_to_display'.

The path "/get-string" displays the latest value of 'string_to_display'.

## Setup

Assumptions:
- rust is installed
- a Telegram bot is created using BotFather and bot token is available.


1. First setup the env variable for the telegram bot
export TELOXIDE_TOKEN='#YOUR_BOT_TOKEN#'

2. Clone the repo. If running locally first, you may want to change the actix web server IP to 127.0.0.1

3. run the application in dev mode using 'cargo run'
