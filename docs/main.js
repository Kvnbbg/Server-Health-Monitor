const copyButton = document.querySelector('#copy-command');
const toggleButton = document.querySelector('#toggle-highlight');
const command = document.querySelector('#vercel-command');

const toggleFocus = () => {
  document.body.classList.toggle('focus');
};

toggleButton?.addEventListener('click', toggleFocus);

copyButton?.addEventListener('click', async () => {
  const text = command?.textContent?.trim();
  if (!text) {
    return;
  }

  try {
    await navigator.clipboard.writeText(text);
    copyButton.textContent = 'Commande copiée ✓';
    setTimeout(() => {
      copyButton.textContent = 'Copier la commande Vercel';
    }, 2000);
  } catch (error) {
    copyButton.textContent = 'Copie impossible';
  }
});
