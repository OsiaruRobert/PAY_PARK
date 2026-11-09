import order from "../order.js";
import park_space from "../park_space.js";

export default async function webhook(req, res) {
    try {

        //Extract values
        if (req.body.event !== "charge.success") throw new Error("Not success");

        console.log("Paid");

        res.send({
            "message": "Good Job"
        });

        const reference = req.body.data.reference;


        order.forEach((item) => {
            if (item.reference === reference) {
                console.log("Paid");
                console.log(`Reference: ${reference}`);
                item.paid = true;
            }
        });

        park_space.forEach((item) => {
            if (item.reference === reference) {
                console.log("Paid");
                item.status = "occupied";
            }
        });


    } catch (err) {
        console.log(`We noticed ${err.message}`);
        res.status(200)
            .json({ "message": "Welcome To PrpjectPQ" });
    }
}