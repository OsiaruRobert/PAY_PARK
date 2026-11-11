import { Router } from "express";
import controllers from "./controllers.js";

const routes = new Router();


routes.get("/", (req, res) => {
    console.log("Request received at /");
    res.json({ "message": "Welcome To Pay Park" })
});


routes.post("/charge", controllers.charge);

routes.get("/paid/:reference", controllers.paid);

routes.post("/webhook", controllers.webhook);
routes.get("/webhook", controllers.webhook);

routes.use((req, res) => {
    res.status(404).json({ "message": "Not Found" });
});

export default routes;