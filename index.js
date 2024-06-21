import mqtt from "mqtt";
import express from "express"
import { createClient } from "@supabase/supabase-js";

const app = express()

const client = mqtt.connect(
    "wss://j727fabd.ala.asia-southeast1.emqxsl.com:8084/mqtt",
    {
      username: "saujanashafi",
      password: "Saujanashafi",
    },
  );
const supabase = createClient("https://fkxwwgpbvohxsepxbbig.supabase.co", "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImZreHd3Z3Bidm9oeHNlcHhiYmlnIiwicm9sZSI6ImFub24iLCJpYXQiOjE3MTgwNzg0MjYsImV4cCI6MjAzMzY1NDQyNn0.OdtU0T79gLCfznEAbP82rWxol9tL6LyQI8z3NQBYcww")

client.on("connect", () => {
    console.log("connected");
    client.subscribe("saujanashafi/esp/gps", err => {
        if (!err) {
            console.log("ok")
        } else {
            console.log(err);
        }
    })
    
    const engineChannels = supabase.channel('custom-update-channel')
    .on(
    'postgres_changes',
    { event: 'UPDATE', schema: 'public', table: 'engine_states' },
    (payload) => {
        console.log(payload.new.is_off);
        if(payload.new.is_off == true) {
            client.publish("saujanashafi/esp/engine", "0")
        } else {
            client.publish("saujanashafi/esp/engine", "1")
        }
    }
    )
    .subscribe()

    const trackingChannels = supabase.channel('custom-update-channel')
    .on(
    'postgres_changes',
    { event: 'UPDATE', schema: 'public', table: 'tracking_states' },
    (payload) => {
        console.log(payload.new.is_tracking);
        if(payload.new.is_tracking == true) {
            client.publish("saujanashafi/esp/start", "1")
        } else {
            client.publish("saujanashafi/esp/start", "0")
        }
    }
    )
    .subscribe()
})

client.on("error", err => {
    console.log(err);
})

client.on("message", async (topic, message) => {
    if (topic == "saujanashafi/esp/gps") {
        const latlang = message.toString().split(",");
        console.log(latlang);
        const supabase = createClient("https://fkxwwgpbvohxsepxbbig.supabase.co", "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImZreHd3Z3Bidm9oeHNlcHhiYmlnIiwicm9sZSI6ImFub24iLCJpYXQiOjE3MTgwNzg0MjYsImV4cCI6MjAzMzY1NDQyNn0.OdtU0T79gLCfznEAbP82rWxol9tL6LyQI8z3NQBYcww")
        const result = await supabase.from("coords").insert({latitude: parseFloat(latlang[0]), longitude: parseFloat(latlang[1])})
        if (result.error) {
            console.log(result);
        } else {
            console.log("successfully inserted");
        }
    }
})

app.get("/", (req, res) => {
    res.send("Hello World")
})

app.listen(8000, () => {
    console.log("Server is running on port 8000")
})