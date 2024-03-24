import { execSync } from 'child_process';
import * as fs from 'fs';

const fonts: [{ name: string; size: number | [number]; fonts: [{ file: string; range: string }] }] = JSON.parse(
  fs.readFileSync('generate-fonts.json', 'utf-8')
);

for (const font of fonts) {
  let sizes = Array.isArray(font.size) ? font.size : [font.size];

  for (const size of sizes) {
    let name = font.name.replace(/\{size\}/g, `${size}`);

    console.log();
    console.log(`Generating ${name}`);
    console.log();

    let args = `--no-compress --no-prefilter --bpp 4 --size ${size} --format lvgl -o "../${name}.c" --force-fast-kern-format --lv-include lvgl.h `;

    for (const file of font.fonts) {
      args += `--font "${file.file}" --range "${file.range}" `;
    }
    
    if (process.platform === "win32") {
        console.log(execSync(`call ./node_modules/.bin/lv_font_conv.cmd ${args}`, { encoding: 'utf-8' }));
    } else {
        console.log(execSync(`./node_modules/.bin/lv_font_conv ${args}`, { encoding: 'utf-8' }));
    }
  }
}
