import order from "../order.js";

export default async function paid(req, res) {
    try {

        const reference = req.params.reference;
        const paidStatus = isPaid(reference);

        res.status(200).json({ ...paidStatus });

    } catch (err) {
        console.log(`We noticed ${err.message}`);
        res.status(200)
            .json({ "message": "Welcome To PAY PARK" });
    }
}

function isPaid(reference) {
    const item = order.find((item) => item.reference === reference);
    return item ? { paid: item.paid, park_space_u: item.park_space_u } : { paid: false, park_space_u: null };
}