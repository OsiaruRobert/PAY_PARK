import order from "./order.js";
import { chargeUser } from "./functions/charge.js";
import webhook from "./functions/webhook.js";
import paid from "./functions/paid.js";
import park_space from "./park_space.js";


const controllers = {
    charge: charge,
    paid: paid,
    webhook: webhook
};

function ranDmT(n = 10) {
    const chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    let result = "";
    for (let i = 0; i < n; i++) {
        const randomIndex = Math.floor(Math.random() * chars.length);
        result += chars[randomIndex];
    }
    return result;
}


async function charge(req, res) {

    try {
        const paymentdetails = await chargeUser(`${ranDmT()}@mail.com`, 100, "car-park");
        res.status(200).json(paymentdetails);

        let park_space_u = false;

        park_space.forEach((item) => {
            if (item.status === "free") {
                item.reference = paymentdetails.reference;
                park_space_u = item.name;
            }
        });

        if (!park_space_u) throw new Error("No free parking space available");

        order.push({
            reference: paymentdetails.reference,

            account_number: paymentdetails.account_number,
            bank_name: paymentdetails.bank_name,
            account_name: paymentdetails.account_name,

            expires_at: paymentdetails.expires_at,
            paid: false,

            park_space_u
        });


    } catch (error) {
        console.error("Error charging user:", error);
        res.status(500).json({ error: "Failed to charge user" });
    }
}




export default controllers;
