const addon = require("./build/Release/addon");
const express = require("express");
const cors = require("cors");

const app = express();
app.use(cors());
app.use(express.json());

// FCFS Endpoint only
app.post("/api/schedule/fcfs", (req, res) => {
  try {
    const result = addon.fcfs(req.body);
    res.json(result);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.post("/api/schedule/sjb", (req, res) => {
  try {
    const result = addon.sjb(req.body);
    res.json(result);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.post("/api/schedule/priornon", (req, res) => {
  try {
    const result = addon.priornon(req.body);
    res.json(result);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.post("/api/schedule/prior", (req, res) => {
  try {
    const result = addon.prior(req.body);
    res.json(result);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.post("/api/schedule/rr", (req, res) => {
  try {
    const { processes, timeQuantum } = req.body;
    const result = addon.rr(processes, timeQuantum);
    res.json(result);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.post("/api/schedule/srtf", (req, res) => {
  try {
    const result = addon.srtf(req.body);
    res.json(result);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

const PORT = 5000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
