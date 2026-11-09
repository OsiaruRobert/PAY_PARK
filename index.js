
import express from "express";
import routes from "./src/routes.js";

const app = express();

const PORT = 3000;

//cors
app.use((req, res, next) => {
    res.setHeader("Access-Control-Allow-Origin", "*");
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    next();
});

app.use(express.json());
app.use(routes);

app.listen(3000, () => {
    console.log(`Running on PORT ${PORT}`);
});
