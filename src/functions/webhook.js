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

        let park_space_name = "";
        park_space.forEach((item) => {
            if (item.reference === reference) {
                console.log("Paid");
                item.status = "occupied";
                park_space_name = item.name;
            }
        });

        await notify(park_space_name);

    } catch (err) {
        console.log(`We noticed ${err.message}`);
        res.status(200)
            .json({ "message": "Welcome To PrpjectPQ" });
    }
}


async function notify(P_S) {
    try {
        const notify_Req = await fetch(`https://robotics.osiaru.top/paid/`, {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                "park_space": P_S
            })
        });
    } catch (err) {
        console.error(`Error notifying park space ${P_S}: ${err.message}`);
    }
}