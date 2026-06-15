import { Router } from "express";
import { promises as fs } from "fs";
import path from "path";

export function avatarsRouter() {
  const router = Router();
  const avatarRoot = path.resolve("../sdcard/avatars");
  router.get("/", async (_req, res) => {
    try {
      const dirs = await fs.readdir(avatarRoot, { withFileTypes: true });
      res.json(dirs.filter(d => d.isDirectory()).map(d => ({ id: d.name, preview: `/api/avatars/${d.name}/1.png` })));
    } catch { res.json([]); }
  });
  router.use("/:id", async (req, res, next) => {
    const file = path.join(avatarRoot, req.params.id, req.path.replace(/^\//, ""));
    res.sendFile(file, err => { if (err) next(); });
  });
  return router;
}
