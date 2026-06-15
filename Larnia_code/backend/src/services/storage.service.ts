import { promises as fs } from "fs";
import path from "path";

export class JsonStore<T> {
  constructor(private filePath: string, private fallback: T) {}
  async get(): Promise<T> {
    try { return JSON.parse(await fs.readFile(this.filePath, "utf8")) as T; }
    catch { await this.set(this.fallback); return this.fallback; }
  }
  async set(value: T): Promise<T> {
    await fs.mkdir(path.dirname(this.filePath), { recursive: true });
    await fs.writeFile(this.filePath, JSON.stringify(value, null, 2), "utf8");
    return value;
  }
}
