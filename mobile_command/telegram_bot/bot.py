import os
import requests

from dotenv import load_dotenv

from telegram import Update
from telegram.ext import (
    ApplicationBuilder,
    CommandHandler,
    ContextTypes,
)

# =========================================================
# ENV
# =========================================================

load_dotenv()

TOKEN = os.getenv("TELEGRAM_BOT_TOKEN")
API_URL = os.getenv("API_URL")

# =========================================================
# AUXILIAR
# =========================================================

def send_api_command(endpoint: str):
    url = f"{API_URL}{endpoint}"

    response = requests.post(url)

    return response.status_code, response.text

# =========================================================
# COMANDOS
# =========================================================

async def start(update: Update, context: ContextTypes.DEFAULT_TYPE):
    await update.message.reply_text(
        "🤖 Drone Bot online.\n\n"
        "Comandos:\n"
        "/arm\n"
        "/offboard\n"
        "/takeoff 2\n"
        "/land\n"
        "/rtl\n"
        "/right 1\n"
        "/left 1\n"
        "/forward 1\n"
        "/back 1\n"
        "/up 1\n"
        "/down 1"
    )

async def arm(update: Update, context: ContextTypes.DEFAULT_TYPE):
    send_api_command("/arm")
    await update.message.reply_text("✅ ARM enviado")

async def offboard(update: Update, context: ContextTypes.DEFAULT_TYPE):
    send_api_command("/offboard")
    await update.message.reply_text("✅ OFFBOARD enviado")

async def land(update: Update, context: ContextTypes.DEFAULT_TYPE):
    send_api_command("/land")
    await update.message.reply_text("🛬 LAND enviado")

async def rtl(update: Update, context: ContextTypes.DEFAULT_TYPE):
    send_api_command("/rtl")
    await update.message.reply_text("🏠 RTL enviado")

async def takeoff(update: Update, context: ContextTypes.DEFAULT_TYPE):
    value = 2.0

    if context.args:
        value = float(context.args[0])

    send_api_command(f"/takeoff?value={value}")

    await update.message.reply_text(
        f"🚀 TAKEOFF {value} m enviado"
    )

async def move(update: Update, context: ContextTypes.DEFAULT_TYPE):
    command = update.message.text.split()[0][1:]

    value = 1.0

    if context.args:
        value = float(context.args[0])

    send_api_command(
        f"/move/{command}?value={value}"
    )

    await update.message.reply_text(
        f"➡️ {command.upper()} {value}"
    )

# =========================================================
# MAIN
# =========================================================

def main():
    app = ApplicationBuilder().token(TOKEN).build()

    app.add_handler(CommandHandler("start", start))

    app.add_handler(CommandHandler("arm", arm))
    app.add_handler(CommandHandler("offboard", offboard))
    app.add_handler(CommandHandler("land", land))
    app.add_handler(CommandHandler("rtl", rtl))

    app.add_handler(CommandHandler("takeoff", takeoff))

    app.add_handler(CommandHandler("right", move))
    app.add_handler(CommandHandler("left", move))
    app.add_handler(CommandHandler("forward", move))
    app.add_handler(CommandHandler("back", move))
    app.add_handler(CommandHandler("up", move))
    app.add_handler(CommandHandler("down", move))

    print("Telegram bot online.")

    app.run_polling()

if __name__ == "__main__":
    main()